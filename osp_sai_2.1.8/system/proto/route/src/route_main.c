
#include "proto.h"
#include "route.h"
#include "fei.h"
#include "lib_netlink.h"
#include "lib_sysctl.h"
#include "arp_api.h"
#include "rt_if.h"
#include "gen/tbl_quagga_debug_define.h"
#include "gen/tbl_quagga_debug.h"
#include "gen/tbl_arpinsp_define.h"
#include "gen/tbl_arpinsp.h"
#include "gen/tbl_arpacl_config_define.h"
#include "gen/tbl_arpacl_config.h"
#include "gen/tbl_arpace_config_define.h"
#include "gen/tbl_arpace_config.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
#include "gen/tbl_ipmc_global_define.h"
#include "gen/tbl_ipmc_global.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "gen/tbl_pim_static_rp_define.h"
#include "gen/tbl_pim_static_rp.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "gen/tbl_nexthop_group_routed_define.h"
#include "gen/tbl_nexthop_group_routed.h"
#include "ipmc.h"
#include "arpinsp.h"
#include "vrrp.h"
#include "arpacl.h"
#include "rdb_hiredis.h"
#include "ipsla_api.h"
#include "track_api.h"
#include "nd_api.h"
#include "nat_api.h"

extern tbl_route_debug_t*
tbl_route_debug_get_route_debug();

extern int32
tbl_route_debug_set_route_debug_field(tbl_route_debug_t *p_rtdbg, tbl_route_debug_field_id_t field_id);

extern int32
tbl_route_debug_dump_one(tbl_route_debug_t *p_rtdbg, tbl_iter_args_t *pargs);
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
    {
        name++;
    }
    else
    {
        name = argv0;
    }

    if (status != 0)
    {
        sal_printf("Try `%s -h' for more information.\n", name);
    }
    else
    {
        sal_printf("Usage: %s [OPTION...]\n", name);
        sal_printf("Daemon which manages routing table and ARP protocol.\n");
        sal_printf("Options:\n");
        sal_printf("\t-v, --version\tPrint version and exit.\n");
        sal_printf("\t-h, --help\tPrint this help and exit.\n");
        sal_printf("\t-s, --sync\tSync data from CDB.\n");
    }
    exit(status);
}

int32
route_sock_init(route_master_t *master)
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

int32
_rt_cmd_get_rt_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_route_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_route_debug_get_route_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_route_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

#define DBG_RT_TYPE_FIELD       0
#define DBG_RT_TYPE_ALL         1

int32
_rt_cmd_process_rt_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_route_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    tbl_route_debug_t *p_db_dbg = NULL;
    field_parse_t field;
    uint32 multi_type = DBG_RT_TYPE_FIELD;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_ROUTE_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_route_debug_get_route_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("ROUTE_DEBUG not found");
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "all"))
            {
                multi_type = DBG_RT_TYPE_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }

        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_RT_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            switch (field_id)
            {
            case TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4:
                GLB_SET_FLAG_COND(value, dbg.route, RTDBG_FLAG_ROUTE_IPV4);
                break;
            case TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6:
                GLB_SET_FLAG_COND(value, dbg.route, RTDBG_FLAG_ROUTE_IPV6);
                break;
            case TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK:
                GLB_SET_FLAG_COND(value, dbg.route, RTDBG_FLAG_ROUTE_NETLINK);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARP_EVENT:
                GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_EVENT);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARP_PACKET:
                GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_PACKET);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_PROTO);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARP_TIMER:
                GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_TIMER);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT:
                GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_EVENT);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET:
                GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_PACKET);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_PROTO);
                break;
            case TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER:
                GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_TIMER);
                break;
            case TBL_ROUTE_DEBUG_FLD_VRRP_ALL:
                GLB_SET_FLAG_COND(value, dbg.vrrp, RTDBG_FLAG_VRRP_ALL);
                break;
            case TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS:
                GLB_SET_FLAG_COND(value, dbg.vrrp, RTDBG_FLAG_VRRP_EVENTS);
                break;
            case TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND:
                GLB_SET_FLAG_COND(value, dbg.vrrp, RTDBG_FLAG_VRRP_PACKET_TX);
                break;
            case TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV:
                GLB_SET_FLAG_COND(value, dbg.vrrp, RTDBG_FLAG_VRRP_PACKET_RX);
                break;
            case TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL:
                GLB_SET_FLAG_COND(value, dbg.vrrp, RTDBG_FLAG_VRRP_PACKET_DETAIL);
                break;
            case TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT:
                GLB_SET_FLAG_COND(value, dbg.ipsla, RTDBG_FLAG_IPSLA_EVENT);
                break;
            case TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET:
                GLB_SET_FLAG_COND(value, dbg.ipsla, RTDBG_FLAG_IPSLA_PACKET);
                break;
            case TBL_ROUTE_DEBUG_FLD_TRACK_EVENT:
                GLB_SET_FLAG_COND(value, dbg.track, RTDBG_FLAG_TRACK_EVENT);
                break;
            case TBL_ROUTE_DEBUG_FLD_ND_EVENT:
                GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_EVENT);
                break;
            case TBL_ROUTE_DEBUG_FLD_ND_PACKET:
                GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_PACKET);
                break;
            case TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL:
                GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_PROTO);
                break;
            case TBL_ROUTE_DEBUG_FLD_ND_TIMER:
                GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_TIMER);
                break;
            case TBL_ROUTE_DEBUG_FLD_NAT_EVENT:
                GLB_SET_FLAG_COND(value, dbg.nat, RTDBG_FLAG_NAT_EVENT);
                break;
            case TBL_ROUTE_DEBUG_FLD_NAT_TIMER:
                GLB_SET_FLAG_COND(value, dbg.nat, RTDBG_FLAG_NAT_TIMER);
			/*add by zhw for ipmc*/
			case TBL_ROUTE_DEBUG_FLD_PIMSM_ALL :
                GLB_SET_FLAG_COND(value, dbg.pimsm, RTDBG_FLAG_PIMSM_ALL);
                break;	
			case TBL_ROUTE_DEBUG_FLD_PIMSM_PIM :
                GLB_SET_FLAG_COND(value, dbg.pimsm, RTDBG_FLAG_PIMSM_PIM);
                break;
			case TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP :
                GLB_SET_FLAG_COND(value, dbg.pimsm, RTDBG_FLAG_PIMSM_IGMP);
                break;		
			case TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER :
                GLB_SET_FLAG_COND(value, dbg.pimsm, RTDBG_FLAG_PIMSM_TIMER);
              /*end add zhw*/
                break;
                
            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_route_debug_set_route_debug_field(&dbg, field_id);
        }
        else if (DBG_RT_TYPE_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.route, RTDBG_FLAG_ROUTE_IPV4);
            GLB_SET_FLAG_COND(value, dbg.route, RTDBG_FLAG_ROUTE_IPV6);
            GLB_SET_FLAG_COND(value, dbg.route, RTDBG_FLAG_ROUTE_NETLINK);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK);

            
            GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_EVENT);
            GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_PACKET);
            GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_PROTO);
            GLB_SET_FLAG_COND(value, dbg.arp, RTDBG_FLAG_ARP_TIMER);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARP_EVENT);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARP_PACKET);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARP_TIMER);

            GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_EVENT);
            GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_PACKET);
            GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_PROTO);
            GLB_SET_FLAG_COND(value, dbg.arpinspection, RTDBG_FLAG_ARPINSPECTION_TIMER);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER);

            GLB_SET_FLAG_COND(value, dbg.vrrp, RTDBG_FLAG_VRRP_ALL);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL);

            GLB_SET_FLAG_COND(value, dbg.ipsla, RTDBG_FLAG_IPSLA_EVENT);
            GLB_SET_FLAG_COND(value, dbg.ipsla, RTDBG_FLAG_IPSLA_PACKET);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET);

            GLB_SET_FLAG_COND(value, dbg.track, RTDBG_FLAG_TRACK_EVENT);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_TRACK_EVENT);

            GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_EVENT);
            GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_PACKET);
            GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_PROTO);
            GLB_SET_FLAG_COND(value, dbg.nd, RTDBG_FLAG_ND_TIMER);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ND_EVENT);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ND_PACKET);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_ND_TIMER);

            GLB_SET_FLAG_COND(value, dbg.nat, RTDBG_FLAG_NAT_EVENT);
            GLB_SET_FLAG_COND(value, dbg.nat, RTDBG_FLAG_NAT_TIMER);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_NAT_EVENT);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_NAT_TIMER);

			/*add by zhw for ipmc*/
			GLB_SET_FLAG_COND(value, dbg.pimsm, RTDBG_FLAG_PIMSM_ALL);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_PIMSM_PIM);	
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_PIMSM_IGMP);
            rc = tbl_route_debug_set_route_debug_field(&dbg, TBL_ROUTE_DEBUG_FLD_PIMSM_TIMER);	
			/*end add zhw*/
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_rt_debug(para, p_node, &field);
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
_rt_cmd_get_quagga_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_quagga_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_quagga_debug_get_quagga_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_quagga_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

#define DBG_TYPE_INVALID        0
#define DBG_TYPE_OSPF_EVENT     1
#define DBG_TYPE_OSPF_ISM       2
#define DBG_TYPE_OSPF_LSA       3
#define DBG_TYPE_OSPF_NSM       4
#define DBG_TYPE_OSPF_PACKET    5
#define DBG_TYPE_OSPF_ZEBRA     6
#define DBG_TYPE_OSPF_ALL       7
#define DBG_TYPE_PIM_ALL        8
#define DBG_TYPE_IGMP_ALL       9
#define DBG_TYPE_MROUTE_ALL     10

int32
_rt_cmd_process_quagga_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_quagga_debug_t dbg;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    uint32 multi_type = DBG_TYPE_INVALID;
    tbl_quagga_debug_t *p_db_dbg = NULL;
    field_parse_t field;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_QUAGGA_DEBUG);
#if 0
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_quagga_debug_get_quagga_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("QUAGGA_DEBUG not found");
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "ospf_all"))
            {
                multi_type = DBG_TYPE_OSPF_ALL;
            }
            else if (0 == sal_strcmp(argv[i], "ospf_zebra_"))
            {
                multi_type = DBG_TYPE_OSPF_ZEBRA;
            }
            else if (0 == sal_strcmp(argv[i], "ospf_ism_"))
            {
                multi_type = DBG_TYPE_OSPF_ISM;
            }
            else if (0 == sal_strcmp(argv[i], "ospf_lsa_"))
            {
                multi_type = DBG_TYPE_OSPF_LSA;
            }
            else if (0 == sal_strcmp(argv[i], "ospf_nsm_"))
            {
                multi_type = DBG_TYPE_OSPF_NSM;
            }
            else if (0 == sal_strcmp(argv[i], "ospf_packet_all"))
            {
                multi_type = DBG_TYPE_OSPF_PACKET;
            }
            else if (0 == sal_strcmp(argv[i], "ospf_event_"))
            {
                multi_type = DBG_TYPE_OSPF_EVENT;
            }
            else if (0 == sal_strcmp(argv[i], "igmp_all"))
            {
                multi_type = DBG_TYPE_IGMP_ALL;
            }
            else if (0 == sal_strcmp(argv[i], "pim_all"))
            {
                multi_type = DBG_TYPE_PIM_ALL;
            }
            else if (0 == sal_strcmp(argv[i], "mroute_all"))
            {
                multi_type = DBG_TYPE_MROUTE_ALL;
            }
            else
            {
                if (0 == sal_strcmp(argv[i], "ospf_packet_ls-ack"))
                {
                    field_id = cdb_get_field_id("ospf_packet_ls_ack", p_node, &p_field);
                }
                else if (0 == sal_strcmp(argv[i], "ospf_packet_ls-request"))
                {
                    field_id = cdb_get_field_id("ospf_packet_ls_request", p_node, &p_field);
                }
                else if (0 == sal_strcmp(argv[i], "ospf_packet_ls-update"))
                {
                    field_id = cdb_get_field_id("ospf_packet_ls_update", p_node, &p_field);
                }
                
                if (field_id < 0)
                {
                    CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
                }
            }
        }
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
        
        if (DBG_TYPE_INVALID == multi_type)
        {        
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
            
            switch (field_id)
            {
            case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ISM_EVENTS);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ISM_STATUS);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ISM_TIMERS);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_FLOODING);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_GENERATE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_INSTALL);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_REFRESH);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSM_EVENTS);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSM_STATUS);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSM_TIMERS);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSSA);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_DD);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_HELLO);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_LS_ACK);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_LS_REQUEST);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_LS_UPDATE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ZEBRA_INTERFACE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_ABR);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_IA);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_ROUTE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF:
                GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_SPF);
                break;
            case TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT:
                GLB_SET_FLAG_COND(value, dbg.igmp, IGMPDBG_FLAG_EVENT);
                break;
            case TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET:
                GLB_SET_FLAG_COND(value, dbg.igmp, IGMPDBG_FLAG_PACKET);
                break;
            case TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE:
                GLB_SET_FLAG_COND(value, dbg.igmp, IGMPDBG_FLAG_TRACE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_MROUTE_ON:
                GLB_SET_FLAG_COND(value, dbg.mroute, MROUTEDBG_FLAG_ON);
                break;
            case TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL:
                GLB_SET_FLAG_COND(value, dbg.mroute, MROUTEDBG_FLAG_DETAIL);
                break;
            case TBL_QUAGGA_DEBUG_FLD_PIM_EVENT:
                GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_EVENT);
                break;
            case TBL_QUAGGA_DEBUG_FLD_PIM_PACKET:
                GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_PACKET);
                break;
            case TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE:
                GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_PACKET_DUMP_RCV);
                break;
            case TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND:
                GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_PACKET_DUMP_SEND);
                break;
            case TBL_QUAGGA_DEBUG_FLD_PIM_TRACE:
                GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_TRACE);
                break;
            case TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA:
                GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_ZEBRA);
                break;

            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, field_id);
        }
        if (DBG_TYPE_OSPF_EVENT == multi_type || DBG_TYPE_OSPF_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT);
            
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_ABR);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_IA);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_ROUTE);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_EVENT_SPF);           
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF);
        }
        if (DBG_TYPE_OSPF_ISM == multi_type || DBG_TYPE_OSPF_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ISM_EVENTS);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ISM_STATUS);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ISM_TIMERS);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS);
        }
        if (DBG_TYPE_OSPF_LSA == multi_type || DBG_TYPE_OSPF_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_FLOODING);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_GENERATE);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_INSTALL);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_LSA_REFRESH);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH);
        }
        if (DBG_TYPE_OSPF_NSM == multi_type || DBG_TYPE_OSPF_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSM_EVENTS);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSM_STATUS);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_NSM_TIMERS);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS);
        }
        if (DBG_TYPE_OSPF_PACKET == multi_type || DBG_TYPE_OSPF_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_DD);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_HELLO);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_LS_ACK);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_LS_REQUEST);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_PACKET_LS_UPDATE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE);
        }
        if (DBG_TYPE_OSPF_ZEBRA == multi_type || DBG_TYPE_OSPF_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ZEBRA_INTERFACE);
            GLB_SET_FLAG_COND(value, dbg.ospf, OSPFDBG_FLAG_ZEBRA_REDISTRIBUTE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE);
        }
        if (DBG_TYPE_IGMP_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.igmp, IGMPDBG_FLAG_EVENT);
            GLB_SET_FLAG_COND(value, dbg.igmp, IGMPDBG_FLAG_PACKET);
            GLB_SET_FLAG_COND(value, dbg.igmp, IGMPDBG_FLAG_TRACE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE);
        }
        if (DBG_TYPE_PIM_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_EVENT);
            GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_PACKET);
            GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_PACKET_DUMP_SEND);
            GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_PACKET_DUMP_RCV);
            GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_TRACE);
            GLB_SET_FLAG_COND(value, dbg.pim, PIMDBG_FLAG_ZEBRA);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_PIM_EVENT);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_PIM_PACKET);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_PIM_TRACE);
            rc = tbl_quagga_debug_set_quagga_debug_field(&dbg, TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA);
        }
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_quagga_debug(para, p_node, &field);
        if (rc < 0)
        {
            CFG_CONFLICT_RET("Invalid path");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
#endif
    return rc;
}

int32
rt_nhg_routed_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t *p_field  = NULL;
    FILE          *fp       = NULL;    
    field_parse_t field;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_nexthop_group_routed_iterate((TBL_ITER_CB_FUNC)tbl_nexthop_group_routed_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rt_nhg_routed_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rt_nhg_routed_cmd_read_session_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
route_init(uint32 sync_enable)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_ROUTED);
    lib_master->task_mst = ctc_task_init();
    route_master_t *master = (route_master_t*)XCALLOC(MEM_PM_ROUTE_MASTER, sizeof(route_master_t));

    if (sync_enable)
    {
        lib_set_sync_enable();
        /* global tbl */
        lib_add_sync_tbl(TBL_SYS_GLOBAL);
        lib_add_sync_tbl(TBL_CHASSIS);
        lib_add_sync_tbl(TBL_ROUTE_GLOBAL);
        lib_add_sync_tbl(TBL_SNMP_CFG);
        lib_add_sync_tbl(TBL_LOG_GLOBAL);
        
        lib_add_sync_tbl(TBL_SYS_SPEC);
        lib_add_sync_tbl(TBL_INTERFACE);
        lib_add_sync_tbl(TBL_ROUTE_IF);
        lib_add_sync_tbl(TBL_IPROUTE_NODE);
        lib_add_sync_tbl(TBL_STATIC_ROUTE_CFG);
        lib_add_sync_tbl(TBL_ARP);
        lib_add_sync_tbl(TBL_ARP_FIB);
        lib_add_sync_tbl(TBL_NEXTHOP);
        lib_add_sync_tbl(TBL_MIRROR);
        lib_add_sync_tbl(TBL_PTP_PORT);
    }
    
    lib_master->proto_mst = master;
    route_sock_init(master);
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    rt_fpm_init();
    rt_fea_init(master);

    ipmc_start();

    cdb_register_cfg_tbl_cb(TBL_ROUTE_DEBUG, _rt_cmd_process_rt_debug);
    cdb_register_cfg_tbl_cb(TBL_QUAGGA_DEBUG, _rt_cmd_process_quagga_debug);
    cdb_register_cfg_tbl_cb(TBL_KERNEL_IF, route_cmd_process_kernel_if);
    cdb_register_cfg_tbl_cb(TBL_ROUTE_IF, rt_if_cmd_process_route_if);
    
    cdb_register_cfg_tbl_cb(TBL_ARP, arp_cmd_process_arp);
    cdb_register_cfg_act_cb(ACT_FLUSH_ARP, arp_cmd_process_flusharp);
    cdb_register_cfg_tbl_cb(TBL_ARP_FIB, arp_cmd_process_arp_fib);    
    cdb_register_cfg_act_cb(ACT_SHOW_ARP, arp_cmd_process_arp_show);
    cdb_register_cfg_tbl_cb(TBL_IPROUTE_NODE, rt_cmd_process_route);
    cdb_register_cfg_tbl_cb(TBL_IPROUTE_NODE_ADD_FAIL, rt_cmd_iproute_node_add_fail_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_IPROUTE_NODE_ADD_FAIL_COUNT, rt_cmd_iproute_node_add_fail_count_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_NEXTHOP, rt_cmd_process_nexthop);
    cdb_register_cfg_tbl_cb(TBL_STATIC_ROUTE_CFG, rt_cmd_process_static_route_cfg);
    cdb_register_cfg_tbl_cb(TBL_STATIC_RT_CNT, rt_cmd_process_static_rt_cnt);
    cdb_pm_subscribe_tbl(TBL_VLAN, arpinsp_cmd_vlan_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_ROUTE_IF, NULL, rt_if_cmd_process_route_if_sync);

    cdb_register_cfg_tbl_cb(TBL_NEXTHOP_GROUP_ROUTED, rt_nhg_routed_cmd_process);

    cdb_register_cfg_tbl_cb(TBL_OSPF, rt_cmd_process_ospf);
    cdb_register_cfg_tbl_cb(TBL_OSPF_NETWORK, rt_cmd_process_ospf_network);
    cdb_register_cfg_tbl_cb(TBL_OSPF_AREA_AUTH, rt_cmd_process_ospf_area_auth);
    cdb_register_cfg_tbl_cb(TBL_OSPF_AREA_RANGE, rt_cmd_process_ospf_area_range);
	cdb_register_cfg_tbl_cb(TBL_OSPF_AREA_STUB, rt_cmd_process_ospf_area_stub);
	cdb_register_cfg_tbl_cb(TBL_OSPF_AREA_NSSA, rt_cmd_process_ospf_area_nssa);
	cdb_register_cfg_tbl_cb(TBL_OSPF_AREA_VLINK, rt_cmd_process_ospf_area_vlink);

	/*add by zhw for ospf6*/
	cdb_register_cfg_tbl_cb(TBL_OSPF6, rt_cmd_process_ospf6);
	cdb_register_cfg_tbl_cb(TBL_OSPF6_AREA_RANGE, rt_cmd_process_ospf6_area_range);
	cdb_register_cfg_tbl_cb(TBL_OSPF6_AREA_STUB, rt_cmd_process_ospf6_area_stub);
	cdb_register_cfg_tbl_cb(TBL_OSPF6_INTERFACE, rt_cmd_process_ospf6_interface);
	/*end add*/
	cdb_register_cfg_tbl_cb(TBL_BGP,rt_cmd_process_bgp);
	cdb_register_cfg_tbl_cb(TBL_BGP_NETWORK,rt_cmd_process_bgp_network);
	cdb_register_cfg_tbl_cb(TBL_BGP_NEIGHBOR,rt_cmd_process_bgp_neighbor);

	cdb_register_cfg_tbl_cb(TBL_RIP,rt_cmd_process_rip);
	cdb_register_cfg_tbl_cb(TBL_RIP_NETWORK,rt_cmd_process_rip_network);
	cdb_register_cfg_tbl_cb(TBL_RIP_NEIGHBOR,rt_cmd_process_rip_neighbor);
	cdb_register_cfg_tbl_cb(TBL_RIP_PASSIVE_IF,rt_cmd_process_rip_passive_if);

	/*add by zhw for ripng*/
	cdb_register_cfg_tbl_cb(TBL_RIPNG,rt_cmd_process_ripng);
	cdb_register_cfg_tbl_cb(TBL_RIPNG_NETWORK,rt_cmd_process_ripng_network);
	cdb_register_cfg_tbl_cb(TBL_RIPNG_PASSIVE_IF,rt_cmd_process_ripng_passive_if);
	/*end add*/

    cdb_register_cfg_tbl_cb(TBL_ROUTE_GLOBAL, route_cmd_process_route_global);

    cdb_register_cfg_tbl_cb(TBL_ECMP_GLOBAL, rt_cmd_process_ecmp_global);
    cdb_register_cfg_tbl_cb(TBL_RR_PREFIX, rt_cmd_process_rr_prefix);
    cdb_register_cfg_act_cb(ACT_SHOW_ECMP, rt_cmd_cmd_show_ecmp);

    rt_sync_init();

    vrrp_start();
    ipsla_start();
    track_start();

    /* support nd modified by liwh for bug 47547, 2018-06-28 */
    nd_start();
    /* liwh end */

    cdb_register_cfg_act_cb(ACT_SHOW_IP_IF_BRIEF, route_cmd_process_show_if_brief);
    cdb_register_cfg_act_cb(ACT_SHOW_IPV6_IF_BRIEF, route_cmd_process_show_v6_if_brief);
    cdb_register_cfg_act_cb(ACT_SHOW_IPV6_ROUTE, route_cmd_process_show_ipv6_route);
    cdb_register_cfg_act_cb(ACT_SHOW_IP_ROUTE, route_cmd_process_show_ip_route);
    cdb_register_cfg_act_cb(ACT_CLEAR_IPV6_ROUTE_STATIC, route_cmd_process_clear_ipv6_route_static);
    cdb_register_cfg_act_cb(ACT_PING_IPV6, route_cmd_process_ping_ipv6_check);
    cdb_register_cfg_act_cb(ACT_PING_MGMT_IPV6, route_cmd_process_ping_mgmt_ipv6_check);
    cdb_register_cfg_act_cb(ACT_TRACEROUTE, route_cmd_process_traceroute_check);
    cdb_register_cfg_act_cb(ACT_TRACEROUTE_IPV6, route_cmd_process_traceroute_ipv6_check);
    cdb_register_cfg_act_cb(ACT_TRACEROUTE_MGMT_IPV6, route_cmd_process_traceroute_mgmt_ipv6_check);
    cdb_register_cfg_act_cb(ACT_TFTP_CHECK, route_cmd_process_tftp_check);
    cdb_register_cfg_act_cb(ACT_FTP_CHECK, route_cmd_process_ftp_check);
    cdb_register_cfg_act_cb(ACT_SSH_CHECK, route_cmd_process_ssh_check);
    cdb_register_cfg_act_cb(ACT_TELNET_CHECK, route_cmd_process_telnet_check);
    cdb_register_cfg_act_cb(ACT_IF_ADDR_CFG_CHECK, if_cmd_process_cfg_check);
    cdb_register_cfg_act_cb(ACT_RT_STATIC_MAX_CHECK, rt_fea_static_max_cfg_check); 
    cdb_register_cfg_act_cb(ACT_CHECK_L3IF_EXIST, if_cmd_process_check_l3if_exist);
    cdb_register_cfg_act_cb(ACT_CHECK_L3IF_ADDR, if_cmd_process_check_l3if_addr);
    cdb_register_cfg_act_cb(ACT_CHECK_MGMT_IF_IP, if_cmd_process_check_mgmt_if_ip);

    lib_load_init();
    arpinsp_start(master);
    arpacl_start(master);

    arp_api_init();
    nat_api_init(master);
    ctc_task_main();
    return PM_E_NONE;
}

int32
main(int32 argc, char **argv)
{
    char *p;
    char *progname;
    uint32 sync_enable = FALSE;
    static const char *shortopts = "shv";
 
/* 
Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
    { "sync",       no_argument,    NULL, 's'},
    { "help",       no_argument,    NULL, 'h'},
    { "version",    no_argument,    NULL, 'v'},
    { 0 }
};
#endif /* HAVE_GETOPT_H */

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
         case 's':
             sync_enable = TRUE;
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
 
    route_init(sync_enable);
    exit(0);
}

