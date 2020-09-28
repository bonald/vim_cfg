#include "hsrv_inc.h"
#include "lib_ioctl.h"

/* modified by liwh for bug 48966, 2019-02-26 */
#include "glb_openflow_define.h"
/* liwh end */

extern uint32 g_ip_icmp_debug;

extern void
ctc_sal_task_dump(int (*show_func)(void *, const char *fmt, ...), void *para);

extern int32
hsrv_openflow_show_openflow(FILE *fp, char **argv, int32 argc);

#ifdef TTPPRODUCT
extern int32
hsrv_openflow_show_ttp_openflow(FILE *fp, char **argv, int32 argc);
#endif

extern int32
hsrv_openflow_second_show_tunnel_port_all(FILE *fp, uint32 type);

extern int32
hsrv_openflow_second_show_tunnel_port_one(FILE *fp, char **argv);

static int32
_hsrv_debug_cmd_get_hsrv_debug(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_hsrv_debug_t* p_dbg = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_dbg = tbl_hsrv_debug_get_hsrv_debug();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_hsrv_debug_dump_one(p_dbg, &args);
    sal_fclose(fp);

    return HSRV_E_NONE;
}

#define DBG_HSRV_TYPE_FIELD       0
#define DBG_HSRV_TYPE_CPU_ALL     1

static int32
_hsrv_debug_cmd_process_hsrv_debug(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_hsrv_debug_t dbg;
    tbl_hsrv_debug_t *p_db_dbg = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    field_parse_t field;
    uint32 multi_type = DBG_HSRV_TYPE_FIELD;
    int32 rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_HSRV_DEBUG);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&dbg, 0, sizeof(dbg));

        /* check exist */
        p_db_dbg = tbl_hsrv_debug_get_hsrv_debug();
        if (!p_db_dbg)
        {
            CFG_CONFLICT_RET("PM_DEBUG not found");
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            if (0 == sal_strcmp(argv[i], "cpu_all"))
            {
                multi_type = DBG_HSRV_TYPE_CPU_ALL;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
            }
        }
        PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

        if (DBG_HSRV_TYPE_FIELD == multi_type)
        {
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }

            switch (field_id)
            {
            case TBL_HSRV_DEBUG_FLD_VSWITCH:
                GLB_SET_FLAG_COND(value, dbg.vswitch, HSRVDBG_FLAG_VSWITCH_VSWITCH);
                break;
            case TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF:
                GLB_SET_FLAG_COND(value, dbg.interface, HSRVDBG_FLAG_INTERFACE_L2IF);
                break;
            case TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF:
                GLB_SET_FLAG_COND(value, dbg.interface, HSRVDBG_FLAG_INTERFACE_L3IF);
                break;
            case TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB:
                GLB_SET_FLAG_COND(value, dbg.interface, HSRVDBG_FLAG_INTERFACE_IFDB);
                break;
            case TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV:
                GLB_SET_FLAG_COND(value, dbg.message, HSRVDBG_FLAG_MESSAGE_PM2HSRV);
                break;
            case TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM:
                GLB_SET_FLAG_COND(value, dbg.message, HSRVDBG_FLAG_MESSAGE_HSRV2PM);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_RX:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RX);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_TX:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_TX);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_BPDU:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BPDU);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_EAPOL:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_LLDP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_LLDP);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_ERPS:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ERPS);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_MACDA:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MACDA);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_RIP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RIP);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_OSPF:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OSPF);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_BGP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BGP);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_ARP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ARP);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_DHCP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_DHCP);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_IPDA:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IPDA);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_IGMP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IGMP);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_OTHER:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OTHER);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_RAW:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RAW);
                break;
            case TBL_HSRV_DEBUG_FLD_CPU_PTP:
                GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_PTP);
                break;
            case TBL_HSRV_DEBUG_FLD_IP_ICMP:
                GLB_SET_FLAG_COND(value, dbg.ip, HSRVDBG_FLAG_IP_ICMP);
                g_ip_icmp_debug = value;
                break;
            case TBL_HSRV_DEBUG_FLD_NEXTHOP:
                GLB_SET_FLAG_COND(value, dbg.nexthop, HSRVDBG_FLAG_NEXTHOP_NEXTHOP);
                break;
                
            case TBL_HSRV_DEBUG_FLD_VLAN:
                GLB_SET_FLAG_COND(value, dbg.vlan, HSRVDBG_FLAG_VLAN_VLAN);
                break;
                
            case TBL_HSRV_DEBUG_FLD_MIRROR:
                GLB_SET_FLAG_COND(value, dbg.mirror, HSRVDBG_FLAG_MIRROR_MIRROR);
                break;
                
            case TBL_HSRV_DEBUG_FLD_FDB:
                GLB_SET_FLAG_COND(value, dbg.fdb, HSRVDBG_FLAG_FDB_FDB);
                break;
                
            case TBL_HSRV_DEBUG_FLD_L2MC:
                GLB_SET_FLAG_COND(value, dbg.l2mc, HSRVDBG_FLAG_L2MC_L2MC);
                break;

            case TBL_HSRV_DEBUG_FLD_AGG:
                GLB_SET_FLAG_COND(value, dbg.agg, HSRVDBG_FLAG_AGG_AGG);
                break;

            case TBL_HSRV_DEBUG_FLD_IPUC:
                GLB_SET_FLAG_COND(value, dbg.ipuc, HSRVDBG_FLAG_IPUC_IPUC);
                break;

            case TBL_HSRV_DEBUG_FLD_NEIGHBOR:
                GLB_SET_FLAG_COND(value, dbg.neighbor, HSRVDBG_FLAG_NEIGHBOR_NEIGHBOR);
                break;

            case TBL_HSRV_DEBUG_FLD_ECMP:
                GLB_SET_FLAG_COND(value, dbg.ecmp, HSRVDBG_FLAG_ECMP_ECMP);
                break;
                
            case TBL_HSRV_DEBUG_FLD_ACL:
                GLB_SET_FLAG_COND(value, dbg.acl, HSRVDBG_FLAG_ACL_ACL);
                break;

            case TBL_HSRV_DEBUG_FLD_STATS:
                GLB_SET_FLAG_COND(value, dbg.stats, HSRVDBG_FLAG_STATS_STATS);
                break;
                
            case TBL_HSRV_DEBUG_FLD_QOS_CLASS:
                GLB_SET_FLAG_COND(value, dbg.qos, HSRVDBG_FLAG_QOS_CLASS);
                break;
            case TBL_HSRV_DEBUG_FLD_QOS_POLICER:
                GLB_SET_FLAG_COND(value, dbg.qos, HSRVDBG_FLAG_QOS_POLICER);
                break;
            case TBL_HSRV_DEBUG_FLD_QOS_PROCESS:
                GLB_SET_FLAG_COND(value, dbg.qos, HSRVDBG_FLAG_QOS_PROCESS);
                break;
            case TBL_HSRV_DEBUG_FLD_QOS_QUEUE:
                GLB_SET_FLAG_COND(value, dbg.qos, HSRVDBG_FLAG_QOS_QUEUE);
                break;
            case TBL_HSRV_DEBUG_FLD_DOT1X:
                GLB_SET_FLAG_COND(value, dbg.dot1x, HSRVDBG_FLAG_DOT1X_DOT1X);
                break;
            case TBL_HSRV_DEBUG_FLD_STP:
                GLB_SET_FLAG_COND(value, dbg.stp, HSRVDBG_FLAG_STP_STP);
                break;
            case TBL_HSRV_DEBUG_FLD_PTP_PKT_RX:
                GLB_SET_FLAG_COND(value, dbg.ptp, HSRVDBG_FLAG_PTP_PKT_RX);
                break;
            case TBL_HSRV_DEBUG_FLD_PTP_PKT_TX:
                GLB_SET_FLAG_COND(value, dbg.ptp, HSRVDBG_FLAG_PTP_PKT_TX);
                break;
            case TBL_HSRV_DEBUG_FLD_PTP_CFG:
                GLB_SET_FLAG_COND(value, dbg.ptp, HSRVDBG_FLAG_PTP_CFG);
                break;
            case TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT:
                GLB_SET_FLAG_COND(value, dbg.ptp, HSRVDBG_FLAG_PTP_INTERRUPT);
                break;
            case TBL_HSRV_DEBUG_FLD_IPMC:
                GLB_SET_FLAG_COND(value, dbg.ipmc, HSRVDBG_FLAG_IPMC_IPMC);
                break;
            /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-11 */
            case TBL_HSRV_DEBUG_FLD_TPOAM_COMMON:
                GLB_SET_FLAG_COND(value, dbg.tpoam, HSRVDBG_FLAG_TPOAM_COMMON);
                break;
            case TBL_HSRV_DEBUG_FLD_TPOAM_EVENT:
                GLB_SET_FLAG_COND(value, dbg.tpoam, HSRVDBG_FLAG_TPOAM_EVENT);
                break;
            case TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE:
                GLB_SET_FLAG_COND(value, dbg.tpoam, HSRVDBG_FLAG_TPOAM_UPDATE);
                break;
            case TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX:
                GLB_SET_FLAG_COND(value, dbg.tpoam, HSRVDBG_FLAG_TPOAM_PKT_RX);
                break;
            case TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX:
                GLB_SET_FLAG_COND(value, dbg.tpoam, HSRVDBG_FLAG_TPOAM_PKT_TX);
                break;
            /* liwh end */

            case TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX:
                GLB_SET_FLAG_COND(value, dbg.l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_RX);
                break;
            case TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX:
                GLB_SET_FLAG_COND(value, dbg.l2protocol, HSRVDBG_FLAG_L2PROTOCOL_PKT_TX);
                break;
            case TBL_HSRV_DEBUG_FLD_NAT_RULE:
                GLB_SET_FLAG_COND(value, dbg.nat, HSRVDBG_FLAG_NAT_RULE);
                break;
            case TBL_HSRV_DEBUG_FLD_NAT_SESSION:
                GLB_SET_FLAG_COND(value, dbg.nat, HSRVDBG_FLAG_NAT_SESSION);
                break;
            case TBL_HSRV_DEBUG_FLD_NAT_TIMER:
                GLB_SET_FLAG_COND(value, dbg.nat, HSRVDBG_FLAG_NAT_TIMER);
                break;
            case TBL_HSRV_DEBUG_FLD_G8032_EVENT:
                GLB_SET_FLAG_COND(value, dbg.g8032, HSRVDBG_FLAG_G8032_EVENT);
                break;

            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
                break;
            }
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, field_id);
        }
        else if (DBG_HSRV_TYPE_CPU_ALL == multi_type)
        {
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RX);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_TX);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BPDU);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_SLOW_PROTO);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_EAPOL);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_LLDP);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ERPS);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MACDA);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RIP);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OSPF);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_BGP);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_ARP);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_DHCP);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IPDA);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_IGMP);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_LIMIT);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_MAC_MISMATCH);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_L3COPY_CPU);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_OTHER);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_RAW);
            GLB_SET_FLAG_COND(value, dbg.cpu, HSRVDBG_FLAG_CPU_TRAFFIC_PTP);
            GLB_SET_FLAG_COND(value, dbg.nat, HSRVDBG_FLAG_G8032_EVENT);
                
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_RX);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_TX);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_BPDU);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_EAPOL);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_LLDP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_ERPS);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_MACDA);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_RIP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_OSPF);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_BGP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_ARP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_DHCP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_IPDA);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_IGMP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_OTHER);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_RAW);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_CPU_PTP);
            rc = tbl_hsrv_debug_set_hsrv_debug_field(&dbg, TBL_HSRV_DEBUG_FLD_G8032_EVENT);
        }
        break;

    case CDB_OPER_GET:
        if ((argc > 0) && (!sal_strcmp(argv[0], "cpu-packet")))
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field));
        }
        else
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        rc = _hsrv_debug_cmd_get_hsrv_debug(para, p_node, &field);
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

int
hsrv_debug_show_to_file(void *fp_para, const char *fmt, ...)
{
    FILE *fp = (FILE *)fp_para;    
    va_list ap;
#define M_DEBUG_OUT_BUFSZ     1024
    char szBuf[M_DEBUG_OUT_BUFSZ];

    va_start(ap, fmt);
    vsnprintf(szBuf, M_DEBUG_OUT_BUFSZ, fmt, ap);
    va_end(ap);
    
    sal_fprintf(fp, "%s", szBuf);    
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_task(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    sal_fprintf(fp, "System SAL Tasks\n");
    sal_task_dump(hsrv_debug_show_to_file, fp);
    sal_fprintf(fp, "\nSDK SAL Tasks\n");    
    ctc_sal_task_dump(hsrv_debug_show_to_file, fp);
    sal_fclose(fp);
    return rc;
}

static int32
hsrv_if_show_link_one(tbl_interface_t* p_if, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs->argv[0];
    char ifname_ext[IFNAME_EXT_SIZE];

    if(p_if->hw_type != GLB_IF_TYPE_PORT_IF && p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
    {
        return HSRV_E_NONE;
    }

    IFNAME_ETH2SHORT(p_if->key.name, ifname_ext);
    sal_fprintf(fp, "%-10s %-14s %-9s %-7s %-5s %-7u 0x%-10"PRIx64" 0x%04x\n",
        ifname_ext,
        /* modified by liwh for bug 37343, 2016-03-22 */
        //cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_if->media),       
        cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_if->phy_type),
        /* liwh end */
        cdb_enum_val2str(glb_port_speed_strs, GLB_PORT_SPEED_MAX, p_if->oper_speed),
        cdb_enum_val2str(glb_port_duplex_strs, GLB_PORT_DUPLEX_MAX, p_if->oper_duplex),
        (GLB_FLAG_ISSET(p_if->flags,GLB_IFF_RUNNING) ? "up" : "down"),
        p_if->ifindex,
        p_if->portid,
        p_if->flags);

    return HSRV_E_NONE;
}

int32
hsrv_if_show_link(FILE *fp)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = fp;

   
    sal_fprintf(fp, "%-10s %-14s %-9s %-7s %-5s %-7s %-10s   %-6s\n", 
        "Interface", "HW Type", "Speed", "Duplex", "Link", "Ifindex", "SAIPort", "Flag");
    sal_fprintf(fp, "----------+--------------+---------+-------+-----+-------+------------+------\n");

    tbl_interface_iterate((TBL_ITER_CB_FUNC)hsrv_if_show_link_one, &args);

    return HSRV_E_NONE;
}


int32
hsrv_debug_show_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    char *ifname = NULL;
    uint32 detail = FALSE;
    (void)ifname;
    (void)detail;
    
    PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (0 == sal_strcmp(argv[1], "port"))
    {
        if (argc > 2)
        {
            ifname = argv[2];
        }
        if (argc > 3)
        {
            detail = TRUE;
        }
        //hsrv_if_show_if(fp, ifname, detail);
    }
    else if (0 == sal_strcmp(argv[1], "agg"))
    {
        //hsrv_if_show_agg(fp);
    }
    else if (0 == sal_strcmp(argv[1], "l3if"))
    {
        //hsrv_if_show_l3if(fp);
    }
    else if (0 == sal_strcmp(argv[1], "link"))
    {
        hsrv_if_show_link(fp);
    }
    else
    {
        sal_fclose(fp);
        CFG_INVALID_PARAM_RET("Invalid interface subtype %s", argv[1]);
    }
    
    sal_fclose(fp);
    return HSRV_E_NONE;
}
#if 0
int32
hsrv_debug_show_nexthop(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_vlan(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    vid_t vid = GLB_ALL_VLAN_ID;
    int32 rc = HSRV_E_NONE;
    uint32 show_port = FALSE;
    
    PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
    
    if (argc > 1)
    {
        if (0 == sal_strcmp(argv[1], "port"))
        {
            show_port = TRUE;
        }
        else
        {
            if (0 != sal_strcmp(argv[1], ""))
            {
                PM_CFG_GET_INTEGER(vid, argv[1], para->p_rs);
            }
            if (argc > 2 && (0 == sal_strcmp(argv[2], "port")))
            {
                show_port = TRUE;
            }
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (show_port)
    {
        //hsrv_vlan_show_vlan_port(fp, vid);
    }
    else
    {
       // hsrv_vlan_show_vlan(fp, vid);
    }
    
    sal_fclose(fp);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_fdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_l2mc(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_storm_control(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_vlan_mapping(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_ipv4(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    uint32 detail = FALSE;

    PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (0 == sal_strcmp(argv[1], "route"))
    {
       // hsrv_ipuc_show_route(fp);
    }
    else if (0 == sal_strcmp(argv[1], "nexthop"))
    {
       // hsrv_ipuc_show_nexthop(fp);
    }
    else if (0 == sal_strcmp(argv[1], "neighbor"))
    {
        detail = (argc > 2) ? TRUE : FALSE;
       // hsrv_neighbor_show_neighbor(fp, detail);
    }
    else
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid ipv4 subtype %s", argv[1]);
    }
    
    sal_fclose(fp);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_ipv6(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_ecmpv4(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_ecmpv6(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_acl(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_qos(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_mpls(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_mplstp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_mplstp_oam(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_tunnel(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_sflow(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_brg(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
   // hsrv_brg_show_brg(fp);
    sal_fclose(fp);

    return rc;
}

int32
hsrv_debug_show_openflow(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    //hsrv_openflow_show_openflow(fp, &argv[1], argc - 1);
    sal_fclose(fp);

    return rc;
}

int32
hsrv_debug_show_tap(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    //hsrv_tap_show_tap(fp, &argv[1], argc - 1);
    sal_fclose(fp);

    return rc;
}
#endif

int32
hsrv_debug_show_message(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_cpu_pdu(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_nexthop(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_vlan(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_mirror(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_fdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_l2mc(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_agg(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_ipuc(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_ecmp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_acl(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_qos(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    CFG_INVALID_PARAM_RET("show hsrv %s is TODO", argv[0]);
    return HSRV_E_NONE;
}

int32
hsrv_debug_show_openflow(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    hsrv_openflow_show_openflow(fp, &argv[1], argc - 1);
    sal_fclose(fp);

    return rc;
}

int32
hsrv_debug_show_ttp_openflow(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = HSRV_E_NONE;

#ifdef TTPPRODUCT
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    hsrv_openflow_show_ttp_openflow(fp, &argv[1], argc - 1);
    sal_fclose(fp);
#endif

    return rc;
}

int32
hsrv_debug_show_sai_opb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    ctclib_opb_dump(fp);
    sal_fclose(fp);

    return rc;
}

int32
_hsrv_debug_show_cmd_process_hsrv_tunnel(char **argv, int32 argc, cfg_cmd_para_t *para)
{   
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if(argc > 0)
    {
        if (argc == 1)
        {
            rc = hsrv_openflow_second_show_tunnel_port_one(fp, argv);
        }
        /* modified by liwh for bug 48966, 2019-02-26 */
        else if ((argc == 2) && (0 == sal_memcmp(argv[0], "brief", 5)))
        {
            if (0 == sal_memcmp(argv[1], "l2gre", 5) )
            {
                rc = hsrv_openflow_second_show_tunnel_port_all(fp, GLB_OPENFLOW_TUNNEL_TYPE_L2GRE);
            }
            else if (0 == sal_memcmp(argv[1], "nvgre", 5) )
            {
                rc = hsrv_openflow_second_show_tunnel_port_all(fp, GLB_OPENFLOW_TUNNEL_TYPE_NVGRE);            
            }
            else if (0 == sal_memcmp(argv[1], "vxlan", 5) )
            {
                rc = hsrv_openflow_second_show_tunnel_port_all(fp, GLB_OPENFLOW_TUNNEL_TYPE_VXLAN);              
            }
        }
        /* liwh end */
    }
    else
    {
        rc = hsrv_openflow_second_show_tunnel_port_all(fp, GLB_OPENFLOW_TUNNEL_TYPE_ALL);
    }
    sal_fclose(fp);
    
    return rc;
}

int32
_hsrv_debug_show_cmd_process_hsrv(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    if (CDB_OPER_GET == para->oper)
    {
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("No show hsrv field parameter");
        }
        
        if (0 == sal_strcmp("task", argv[0]))
        {
            return hsrv_debug_show_task(argv, argc, para);
        }
        else if (0 == sal_strcmp("message", argv[0]))
        {
            return hsrv_debug_show_message(argv, argc, para);
        }
        else if (0 == sal_strcmp("interface", argv[0]))
        {
            return hsrv_debug_show_interface(argv, argc, para);
        }
        else if (0 == sal_strcmp("agg", argv[0]))
        {
            return hsrv_debug_show_agg(argv, argc, para);
        }
        else if (0 == sal_strcmp("nexthop", argv[0]))
        {
            return hsrv_debug_show_nexthop(argv, argc, para);
        }
        else if (0 == sal_strcmp("vlan", argv[0]))
        {
            return hsrv_debug_show_vlan(argv, argc, para);
        }
        else if (0 == sal_strcmp("fdb", argv[0]))
        {
            return hsrv_debug_show_fdb(argv, argc, para);
        }
        else if (0 == sal_strcmp("l2mc", argv[0]))
        {
            return hsrv_debug_show_l2mc(argv, argc, para);
        }
        else if (0 == sal_strcmp("acl", argv[0]))
        {
            return hsrv_debug_show_acl(argv, argc, para);
        }
        else if (0 == sal_strcmp("qos", argv[0]))
        {
            return hsrv_debug_show_qos(argv, argc, para);
        }
        else if (0 == sal_strcmp("ipuc", argv[0]))
        {
            return hsrv_debug_show_ipuc(argv, argc, para);
        }
        else if (0 == sal_strcmp("openflow", argv[0]))
        {
            return hsrv_debug_show_openflow(argv, argc, para);
        }
        else if (0 == sal_strcmp("mdttp_openflow", argv[0]))
        {
            return hsrv_debug_show_ttp_openflow(argv, argc, para);
        }
        else if (0 == sal_strcmp("sai_opb", argv[0]))
        {
            return hsrv_debug_show_sai_opb(argv, argc, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Unhandled field %s", argv[0]);
        }
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid action");
    }
    
    return rc;
}
    
int32
_hsrv_debug_clear_cmd_process_hsrv(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;
    int32 rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_act(ACT_CLEAR_HSRV);
    
    if (CDB_OPER_SET == para->oper)
    {
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("No show hsrv field parameter");
        }
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
 
        switch (field_id)
        {
        case 0:

        default:
            CFG_INVALID_PARAM_RET("Unhandled field %s", argv[0]);
        }
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid action");
    }
    
    return rc;
}

int32
hsrv_debug_packets_tx(char *buffer, int32 buffer_size, uint32 pdu_type)
{
    tbl_cpu_packets_t *cpu_stats = NULL;
    
    cpu_stats = tbl_cpu_packets_get_cpu_packets();

    switch (pdu_type)
    {
    case GLB_PKT_IGMP:
        ++cpu_stats->tx_igmp;
        break;
    case GLB_PKT_BPDU:
        ++cpu_stats->tx_bpdu;
        break;
    case GLB_PKT_LACP:
    case GLB_PKT_SSM: /*added by yejl for clock synce*/
        ++cpu_stats->tx_slow_proto;
        break;
    case GLB_PKT_ARP:
        ++cpu_stats->tx_arp;
        break;
    case GLB_PKT_DHCP:
        ++cpu_stats->tx_dhcp;
        break;
    case GLB_PKT_L2_LLDP:
        ++cpu_stats->tx_lldp;
        break;
    case GLB_PKT_ERPS:
        ++cpu_stats->tx_erps;
        break;
    case GLB_PKT_EAPOL:
        ++cpu_stats->tx_eapol;
        break;
    case GLB_PKT_PTP:
        ++cpu_stats->tx_ptp;
        break;
    case GLB_PKT_TPOAM:
        ++cpu_stats->tx_tpoam;
        break;
    case GLB_PKT_L2_PROTOCOL_MAC:
        ++cpu_stats->tx_l2pro_protocol_mac;
        break;
    case GLB_PKT_L2_GROUP_MAC:
        ++cpu_stats->tx_l2pro_group_mac;
        break;
    case GLB_PKT_CPU_MIRROR:
        ++cpu_stats->tx_cpu_mirror;
        break;
    case GLB_PKT_ND:
        ++cpu_stats->tx_nd;
        break;
    case GLB_PKT_RAPS:
        ++cpu_stats->tx_g8032;      
        break;
    default:
        ++cpu_stats->tx_other;
        break;
    }
    ++cpu_stats->total;
    ++cpu_stats->tx;

    if (!HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_TX))
    {
        return HSRV_E_NONE;
    }

    /* Tx packet not contain reason information */
    HSRV_CPU_TRAFFIC_OTHER_DEBUG("TX packet, pdu_type: %u\n", pdu_type);

    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RAW))
    {
        int i;
        int count = 0;
        int32 buffer_size_tmp = 0;
        char buf[543];
        memset(buf, 0, sizeof buf);
        count += snprintf(buf + count, sizeof buf - count, "Raw:");
        /*added by yejl for bug 52106, 2019-07-30*/
        if (buffer_size % 2 == 0)
        {
            buffer_size_tmp = buffer_size;
        }
        else
        {
            buffer_size_tmp = buffer_size - 1;     
        }
        /*ended*/
        for (i = 0 ; i < buffer_size_tmp ; ++i) {
            if (count > sizeof buf) break;
            if (!(i % 16)) {
                /* The syslog-ng we used not permit multi-line log msg, every
                 * newline character present a split new log msg */
                count += snprintf(buf + count, sizeof buf - count, " %02hhx", buffer[i]);
            } else {
                count += snprintf(buf + count, sizeof buf - count, "%02hhx", buffer[i]);
            }
        }
        log_sys(M_MOD_HSRV, E_DEBUG, "%s", buf);
    }

    return HSRV_E_NONE;
}

int32
hsrv_debug_packets_rx(char *buffer, int32 buffer_size, int32 reason, char* ifname_ext, int32 src_svid)
{
    tbl_cpu_packets_t *cpu_stats = NULL;
    
    cpu_stats = tbl_cpu_packets_get_cpu_packets();

    ++cpu_stats->total;
    ++cpu_stats->rx;

    switch (reason)
    {
        /* l2 pdu */
    case SAI_HOSTIF_TRAP_ID_STP:
        ++cpu_stats->rx_bpdu;
        break;
    case SAI_HOSTIF_TRAP_ID_LACP:
        ++cpu_stats->rx_slow_proto;
        break;
    case SAI_HOSTIF_TRAP_ID_EAPOL:
        ++cpu_stats->rx_eapol;
        break;
    case SAI_HOSTIF_TRAP_ID_LLDP:
        ++cpu_stats->rx_lldp;
        break;
    case SAI_HOSTIF_TRAP_ID_ERPS:
        ++cpu_stats->rx_erps;
        break;
    /** port_security macnum exceeds the maxnum
     * (default packet action is trap) */
    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT:
        ++cpu_stats->rx_mac_limit;
        break;
    /** port_security mac mismatch with other port
     * (default packet action is trap) */
    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_MISMATCH:
        ++cpu_stats->rx_mac_mismatch;
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L3_COPY_CPU:
        ++cpu_stats->rx_l3copy_cpu;
        break;
#if 0
    case CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_MACDA:
        ++cpu_stats->rx_macda;
        break;
        /* l3 pdu */
    case CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_RIP:
        ++cpu_stats->rx_rip;
        break;
#endif
    case SAI_HOSTIF_TRAP_ID_OSPF:
        ++cpu_stats->rx_ospf;
        break;
    case SAI_HOSTIF_TRAP_ID_BGP:
        ++cpu_stats->rx_bgp;
        break;
    case SAI_HOSTIF_TRAP_ID_ARP_REQUEST:
    case SAI_HOSTIF_TRAP_ID_ARP_RESPONSE:
        ++cpu_stats->rx_arp;
        break;
    case SAI_HOSTIF_TRAP_ID_DHCP:
        ++cpu_stats->rx_dhcp;
        break;
#if 0
    case CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_IPDA:
        ++cpu_stats->rx_ipda;
        break;
#endif
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT:
        ++cpu_stats->rx_igmp;
        break;
    case SAI_HOSTIF_TRAP_ID_TTL_ERROR:
        ++cpu_stats->rx_ttl_error;
        break;
    case SAI_HOSTIF_TRAP_ID_L3_MTU_ERROR:
        ++cpu_stats->rx_mtu_fail;
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP:
        ++cpu_stats->rx_ptp;
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_TPOAM:
        ++cpu_stats->rx_tpoam;
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_PROTOCOL_MAC:
        ++cpu_stats->rx_l2pro_protocol_mac;
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_GROUP_MAC:
        ++cpu_stats->rx_l2pro_group_mac;
        break;
    case SAI_HOSTIF_TRAP_ID_CPU_MIRROR:
        ++cpu_stats->rx_cpu_mirror;
        break;
    case SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY:
        ++cpu_stats->rx_nd;
        break;
    case SAI_HOSTIF_TRAP_ID_L2_COPY_TO_CPU:
        ++cpu_stats->rx_g8032;      
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_PACKET_IN:
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_TO_CONTROLLER:
        ++cpu_stats->rx_packetin;
        break;
    default:
        ++cpu_stats->rx_other;
        break;
    }
    
    if (!HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RX))
    {
        return HSRV_E_NONE;
    }

    switch (reason)
    {
        /* l2 pdu */
    case SAI_HOSTIF_TRAP_ID_STP:
        HSRV_CPU_TRAFFIC_BPDU_DEBUG("RX BPDU, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_LACP:
        HSRV_CPU_TRAFFIC_SLOW_PROTO_DEBUG("RX Slow Proto, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_EAPOL:
        HSRV_CPU_TRAFFIC_EAPOL_DEBUG("RX EAPOL, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_LLDP:
        HSRV_CPU_TRAFFIC_LLDP_DEBUG("RX LLDP, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_ERPS:
        HSRV_CPU_TRAFFIC_ERPS_DEBUG("RX ERPS, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    /** port_security macnum exceeds the maxnum
     * (default packet action is trap) */
    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT:
        HSRV_CPU_TRAFFIC_MAC_LIMIT_DEBUG("RX MAC LIMIT, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    /** port_security mac mismatch with other port
     * (default packet action is trap) */
    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_MISMATCH:
        HSRV_CPU_TRAFFIC_MAC_MISMATCH_DEBUG("RX MAC MISMATCH, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L3_COPY_CPU: 
        HSRV_CPU_TRAFFIC_L3COPY_CPU_DEBUG("RX L3 COPY CPU, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
#if 0
    case CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_MACDA:
        HSRV_CPU_TRAFFIC_MACDA_DEBUG("RX MACDA, in_port: %u, reason: %d, vlan: %d\n", src_port, reason, src_svid);
        break;
        /* l3 pdu */
    case CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_RIP:
        HSRV_CPU_TRAFFIC_RIP_DEBUG("RX RIP, in_port: %u, reason: %d, vlan: %d\n", src_port, reason, src_svid);
        break;
#endif
    case SAI_HOSTIF_TRAP_ID_OSPF:  
        HSRV_CPU_TRAFFIC_OSPF_DEBUG("RX OSPF, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_BGP:
        HSRV_CPU_TRAFFIC_BGP_DEBUG("RX BGP, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_ARP_REQUEST:
    case SAI_HOSTIF_TRAP_ID_ARP_RESPONSE:
        HSRV_CPU_TRAFFIC_ARP_DEBUG("RX ARP, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_DHCP:
        HSRV_CPU_TRAFFIC_DHCP_DEBUG("RX DHCP, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
#if 0
    case CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_IPDA:
        HSRV_CPU_TRAFFIC_IPDA_DEBUG("RX IPDA, in_port: %u, reason: %d, vlan: %d\n", src_port, reason, src_svid);
        break;
#endif
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT:
        HSRV_CPU_TRAFFIC_IGMP_DEBUG("RX IGMP, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP:
        HSRV_CPU_TRAFFIC_PTP_DEBUG("RX PTP, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    default:
        HSRV_CPU_TRAFFIC_OTHER_DEBUG("RX Other, in_port: %s, reason: %d, vlan: %d\n", ifname_ext, reason, src_svid);
        break;
    }

    if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RAW))
    {
        int i;
        int count = 0;
        char buf[543];  /* 256 bytes * 2  + 31 spaces*/
        memset(buf, 0, sizeof buf);
        count += snprintf(buf + count, sizeof buf - count, "Raw:");
        for (i = 0 ; i < buffer_size; ++i) {
            if (count > sizeof buf) break;

            if (!(i % 16)) {
                /* The syslog-ng we used not permit multi-line log msg, every
                 * newline character present a split new log msg */
                count += snprintf(buf + count, sizeof buf - count, " %02hhx", buffer[i]);
            } else {
                count += snprintf(buf + count, sizeof buf - count, "%02hhx", buffer[i]);
            }
        }
        log_sys(M_MOD_HSRV, E_DEBUG, "%s", buf);
    }

    return HSRV_E_NONE;
}

int32
hsrv_debug_start()
{
    cdb_register_cfg_tbl_cb(TBL_HSRV_DEBUG, _hsrv_debug_cmd_process_hsrv_debug);

    cdb_register_cfg_act_cb(ACT_SHOW_HSRV, _hsrv_debug_show_cmd_process_hsrv);
    cdb_register_cfg_act_cb(ACT_SHOW_HSRV_TUNNEL, _hsrv_debug_show_cmd_process_hsrv_tunnel);
    cdb_register_cfg_act_cb(ACT_CLEAR_HSRV, _hsrv_debug_clear_cmd_process_hsrv);
    return HSRV_E_NONE;
}

