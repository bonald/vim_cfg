#include "proto.h"
#include "cdbclient.h"
#include "glb_l2_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_brg_global_define.h"
#include "mstp_define.h"

static char *mstp_version_str[] = 
{
    "STP",
    "Invalid",
    "RSTP",
    "MSTP"
};

char*
cdbclient_brgid_2_pri_addr(char *brgid)
{
    char *p = NULL;
    p = sal_strchr(brgid, CMD_KEY_DIV_CHAR);
    if (p)
    {
        p[0] = CMD_ZERO_CHAR;
        p++;
    }
    
    return p;
}

char*
cdbclient_brgid_2_format(char *brgid)
{
    char *p = NULL;
    p = sal_strchr(brgid, CMD_KEY_DIV_CHAR);
    if (p)
    {
        p[0] = '-';
        p++;
    }
    
    return p;
}

static int32
_cdbctl_show_mstp_port_brief_one(char *line, uint32 is_stp)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    char *ifname = NULL;
    uint32 oper_p2p = FALSE;
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;
    char *port_state = NULL;
    uint32 oper_rootguard = FALSE;
    /* modified by liwh for bug 37762, 2016-06-21 */
    uint32 oper_loopguard = FALSE;
    /* liwh end */
        
    cfg_get_parse(line, &info, 0);

    ifname = info.value[TBL_MSTP_PORT_FLD_KEY];
    IFNAME_ETH2FULL(ifname, ifname_ext);

    if (0 == sal_strcmp(info.value[TBL_MSTP_PORT_FLD_PORT_ENABLE], "0"))
    {
        sal_printf("%-12s Port protocol disabled\n", ifname_ext);
        return PM_E_NONE;
    }

    if (0 == sal_strcmp(info.value[TBL_MSTP_PORT_FLD_PORTENABLED], "0"))
    {
        return PM_E_NONE;
    }

    oper_p2p = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPERP2PMAC], &rc);
    port_state = info.value[TBL_MSTP_PORT_FLD_PST_STATE];

    /* modified by liwh for bug 37762, 2016-06-21 */
    oper_loopguard = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPER_LOOPGUARD], &rc);
    /* liwh end */
    
    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    oper_rootguard      = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPER_ROOTGUARD], &rc);

    if (oper_rootguard && ((!sal_strcmp("Discarding", port_state))
        || (!sal_strcmp("Listening", port_state))))
    {
        port_state = "Root-inconsistent";
    }
    /* liwh end */
    /* modified by liwh for bug 37762, 2016-06-21 */
    else if (oper_loopguard && (!sal_strcmp("Discarding", port_state))
        && (!sal_strcmp("Designated", info.value[TBL_MSTP_PORT_FLD_ROLE])))
    {
        port_state = "Loop-inconsistent";    
    }
    /* liwh end */
    else if (is_stp && (!sal_strcmp("Discarding", port_state)))
    {
        port_state = "Blocked";
    }

    sal_printf("%-12s %-12s %-17s %-10s %9s.%-8s %-6s\n", 
        ifname_ext,
        info.value[TBL_MSTP_PORT_FLD_ROLE],
        port_state,
        info.value[TBL_MSTP_PORT_FLD_PORTPATHCOST],
        info.value[TBL_MSTP_PORT_FLD_PRIORITY],
        info.value[TBL_MSTP_PORT_FLD_IFINDEX],
        oper_p2p ? "P2p" : "Shared");
    return PM_E_NONE;
}

static int32
_cdbctl_show_mstp_port_brief(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info, uint32 is_stp)
{
    uint32 banner = TRUE;
    char  line[FILE_READ_LINE_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        if (banner)
        {
            sal_printf("%-12s %-12s %-17s %-10s %9s.%-8s %-6s\n", "Interface", "Role", "State", "Cost", "Priority", "Number", "Type");
            sal_printf("------------+------------+-----------------+--------------------+--------+------\n");
            banner = FALSE;
        }
        _cdbctl_show_mstp_port_brief_one(line, is_stp);
    }

    return PM_E_NONE;
}

static const char *mstp_edgeport_show_strs[3] =
{
    "Disabled",
    "Enabled",
    "Default"
};

const char *mstp_link_strs[3] =
{
    "shared",
    "p2p",
    "auto"
};

const char *mstp_link_show_strs[3] =
{
    "Shared",
    "P2p",
    "Auto"
};

#define PORT_WIDTH_STR "%-13s"

static int32
_cdbctl_show_mstp_port_one(char *line, uint32 is_stp)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    char *ifname = NULL;
    cfg_parse_get_info_t info;
    uint32 portId = 0;
    uint32 designatedPortId = 0;
    uint32 designated_priority = 0;
    uint32 forceVersion = 0;
    uint32 rcvdStp = FALSE;
    uint32 rcvdRstp = FALSE;
    uint32 sendRstp = FALSE;
    uint32 adminPortPathCost = 0;
    uint32 adminEdge = FALSE;
    uint32 operEdge = FALSE;
    uint32 operP2PMac = FALSE;
    uint32 oper_bpduguard = FALSE;
    uint32 oper_bpdufilter = FALSE;
    uint32 admin_bpduguard = 0;
    uint32 admin_bpdufilter = 0;
    uint32 admin_p2pmac = 0;
    uint32 admin_rootguard = FALSE;
    uint32 oper_rootguard = FALSE;
    uint32 admin_loopguard = FALSE;
    uint32 oper_loopguard = FALSE;
    char *designated_root_str = NULL;
    char *designated_brg_str = NULL;
    char *port_state = NULL;
    int32 rc = PM_E_NONE;
        
    cfg_get_parse(line, &info, 0);

    ifname              = info.value[TBL_MSTP_PORT_FLD_KEY];
    portId              = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_PORTID], &rc);
    designatedPortId    = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDPORTID], &rc);
    forceVersion        = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_FORCEVERSION], &rc);
    rcvdStp             = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_RCVDSTP], &rc);
    rcvdRstp            = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_RCVDRSTP], &rc);
    sendRstp            = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_SENDRSTP], &rc);
    adminEdge           = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_ADMINEDGE], &rc);
    operEdge            = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPEREDGE], &rc);
    operP2PMac          = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPERP2PMAC], &rc);
    oper_bpduguard      = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPER_BPDUGUARD], &rc);
    oper_bpdufilter     = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPER_BPDUFILTER], &rc);
    adminPortPathCost   = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST], &rc);
    admin_rootguard     = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD], &rc);
    oper_rootguard      = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPER_ROOTGUARD], &rc);
    admin_loopguard     = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD], &rc);
    oper_loopguard      = cdb_int_str2val(info.value[TBL_MSTP_PORT_FLD_OPER_LOOPGUARD], &rc);
    admin_bpduguard     = cdb_enum_str2val(mstp_edgeport_strs, 3, info.value[TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD]);
    admin_bpdufilter    = cdb_enum_str2val(mstp_edgeport_strs, 3, info.value[TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER]);
    admin_p2pmac        = cdb_enum_str2val(mstp_link_strs, 3, info.value[TBL_MSTP_PORT_FLD_ADMINP2PMAC]);
    designated_priority = ((designatedPortId & 0xF000) >> 12) * 16;

    designated_root_str = info.value[TBL_MSTP_PORT_FLD_DESIGNATED_ROOTID];
    designated_brg_str = info.value[TBL_MSTP_PORT_FLD_DESIGNATED_DESIGNATEDBRIDGEID];
    cdbclient_brgid_2_format(designated_root_str);
    cdbclient_brgid_2_format(designated_brg_str);
    
    IFNAME_ETH2FULL(ifname, ifname_ext);

    if (0 == sal_strcmp(info.value[TBL_MSTP_PORT_FLD_PORT_ENABLE], "0"))
    {
        sal_printf(" %s: Port protocol disabled", ifname_ext);
        return PM_E_NONE;
    }

    port_state = info.value[TBL_MSTP_PORT_FLD_PST_STATE];

    /* modified by liwh for bug 36699/37625/37767, 2016-05-11 */
    if (oper_rootguard && ((!sal_strcmp("Discarding", port_state))
        || (!sal_strcmp("Listening", port_state))))
    {
        port_state = "Root-inconsistent";
    }
    /* liwh end */
    /* modified by liwh for bug 37762, 2016-06-21 */
    if (oper_loopguard && (!sal_strcmp("Discarding", port_state))
        && (!sal_strcmp("Designated", info.value[TBL_MSTP_PORT_FLD_ROLE])))
    {
        port_state = "Loop-inconsistent";
    }
    /* liwh end */
    else if (is_stp && (!sal_strcmp("Discarding", port_state)))
    {
        port_state = "Blocked";
    }    
    
    sal_printf("-----[Interface %s][Role %s][State %s]-----\n", 
        ifname_ext,
        info.value[TBL_MSTP_PORT_FLD_ROLE],
        port_state);
    
    sal_printf(" %s: "PORT_WIDTH_STR" - Index %s, Priority %s, Id %X\n",
        ifname_ext,
        "Port",
        info.value[TBL_MSTP_PORT_FLD_IFINDEX],
        info.value[TBL_MSTP_PORT_FLD_PRIORITY],
        portId);
#if 0
  if (IS_BRIDGE_MSTP (br))
    cli_out (cli, " %s: Designated External Path Cost %u -"
             "Internal Path Cost %u  \n",
             ifname,
             port->cist_external_rpc,
             port->cist_internal_rpc);
   else
     cli_out (cli, " %s: Designated Path Cost %u\n",
              ifname,
              port->cist_external_rpc);
#endif

    if (adminPortPathCost)
    {
        sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s \n",
            ifname_ext,
            "Port PathCost",
            info.value[TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST],
            info.value[TBL_MSTP_PORT_FLD_PORTPATHCOST]);
    }
    else
    {
        sal_printf(" %s: "PORT_WIDTH_STR" - admin None, oper %s \n",
            ifname_ext,
            "Port PathCost",
            info.value[TBL_MSTP_PORT_FLD_PORTPATHCOST]);
    }

    sal_printf(" %s: "PORT_WIDTH_STR" - HelloTime %s, MaxAge %s, ForwardDelay %s, MessageAge %s\n",
        ifname_ext,
        "Port Time",
        info.value[TBL_MSTP_PORT_FLD_PORT_HELLOTIME],
        info.value[TBL_MSTP_PORT_FLD_PORT_MAXAGE],
        info.value[TBL_MSTP_PORT_FLD_PORT_FWDDELAY],
        info.value[TBL_MSTP_PORT_FLD_PORT_MESSAGEAGE]);

    sal_printf(" %s: "PORT_WIDTH_STR" - Hello %d, ForwardDelay %d, MessageAge %d, TopoChange %d\n",
        ifname_ext,
        "Port Timer",
        cdbclt_timer_get_remain_seconds(info.value[TBL_MSTP_PORT_FLD_HELLOWHEN_TIMER]),
        cdbclt_timer_get_remain_seconds(info.value[TBL_MSTP_PORT_FLD_FDWHILE_TIMER]),
        cdbclt_timer_get_remain_seconds(info.value[TBL_MSTP_PORT_FLD_RCVDINFOWHILE_TIMER]),
        cdbclt_timer_get_remain_seconds(info.value[TBL_MSTP_PORT_FLD_TCWHILE_TIMER]));

    sal_printf(" %s: "PORT_WIDTH_STR" - Root %s, Bridge %s\n",
        ifname_ext,
        "Designated",
        designated_root_str,
        designated_brg_str);

    sal_printf(" %s: "PORT_WIDTH_STR" - Port Id %X, Priority %u, Path Cost %s\n",
        ifname_ext,
        "Designated",
        designatedPortId,
        designated_priority,
        info.value[TBL_MSTP_PORT_FLD_DESIGNATED_ROOTPATHCOST]);

    sal_printf(" %s: "PORT_WIDTH_STR" - Send %s, Received %s\n",
        ifname_ext,
        "BPDU Count",
        info.value[TBL_MSTP_PORT_FLD_TX_BPDU_COUNT],
        info.value[TBL_MSTP_PORT_FLD_RX_BPDU_COUNT]);

    sal_printf(" %s: "PORT_WIDTH_STR" - %s, Send %s, Received %s\n",
        ifname_ext,
        "BPDU Version",
        mstp_version_str[forceVersion],
        sendRstp ? "RSTP" : "STP",
        rcvdRstp ? "RSTP" : rcvdStp ? "STP" : "None");

    sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
        ifname_ext,
        "Edgeport",
        enabled_str(adminEdge),
        on_off_str(operEdge));

    sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
        ifname_ext,
        "BPDU Filter",
        mstp_edgeport_show_strs[admin_bpdufilter],
        on_off_str(oper_bpdufilter));

    sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
        ifname_ext,
        "BPDU Guard",
        mstp_edgeport_show_strs[admin_bpduguard],
        on_off_str(oper_bpduguard));

    sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
        ifname_ext,
        "Root Guard",
        enabled_str(admin_rootguard),
        on_off_str(oper_rootguard));

    sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
        ifname_ext,
        "Loop Guard",
        enabled_str(admin_loopguard),
        on_off_str(oper_loopguard));

    sal_printf(" %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
        ifname_ext,
        "Link Type",
        mstp_link_show_strs[admin_p2pmac],
        operP2PMac ? "P2p" : "Shared");

    sal_printf(" %s: Forward-transitions %s\n",
        ifname_ext,
        info.value[TBL_MSTP_PORT_FLD_FORWARD_TRANSITIONS]);
    
    sal_printf("\n");
    
    return PM_E_NONE;
}

int32
cdbclt_timer_get_remain_seconds(char *str)
{
    char *p = NULL;
    char time_str[CMD_BUF_64];
    int32 value = 0;
    int32 rc = PM_E_NONE;

    time_str[0] = CMD_ZERO_CHAR;
    if (NULL == str)
    {
        return 0;
    }
    else if (NULL == sal_strstr(str, "timer#"))
    {
        return 0;
    }
    else
    {
        p = str + sal_strlen("timer#");
        sal_strncpy(time_str, p, CMD_BUF_64);
        p = sal_strchr(time_str, '.');
        if (p)
        {
            p[0] = CMD_ZERO_CHAR;
        }
        value = cdb_int_str2val(time_str, &rc);
        return value;
    }

    return 0;
}

static int32
_cdbctl_show_mstp_port(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info, uint32 is_stp)
{
    char  line[FILE_READ_LINE_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_mstp_port_one(line, is_stp);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_mstp_disabled_port(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];

    sal_printf("%-16s\n", "Interface");
    sal_printf("----------------\n");
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        IFNAME_ETH2FULL(info.value[TBL_MSTP_PORT_FLD_KEY], ifname_ext);
        sal_printf("%-16s\n", ifname_ext);
    }

    return PM_E_NONE;
}

uint32
cdbctl_is_stp()
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    uint32 is_stp = FALSE;
    int32 rc = PM_E_NONE;
    
    rc = cdbclt_get_tbl_line(TBL_MSTP_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN);
    if (rc < 0)
    {
        return is_stp;
    }
    rc = cfg_get_parse(line, &info, 0);
    if (rc < 0)
    {
        return is_stp;
    }
    
    if (0 == sal_strcmp("STP", info.value[TBL_MSTP_GLOBAL_FLD_TYPE]))
    {
        is_stp = TRUE;
    }

    return is_stp;
}

int32
cdbctl_show_mstp_port(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *field_str = (char*)p_data;
    uint32 is_stp = FALSE;
    
    if (NULL == field_str)
    {
        return PM_E_FAIL;
    }

    is_stp = cdbctl_is_stp();

    if (sal_strstr(field_str, "brief"))
    {
        _cdbctl_show_mstp_port_brief(p_data, data_len, fp, p_info, is_stp);
    }
    else if (sal_strstr(field_str, "up"))
    {
        _cdbctl_show_mstp_port(p_data, data_len, fp, p_info, is_stp);
    }
    else if (sal_strstr(field_str, "all"))
    {
        _cdbctl_show_mstp_port(p_data, data_len, fp, p_info, is_stp);
    }
    else if (sal_strstr(field_str, "disabled"))
    {
        _cdbctl_show_mstp_disabled_port(p_data, data_len, fp, p_info);
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_mstp_global_mst(FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];

    if (NULL == sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        return PM_E_NONE;
    }
    
    cfg_get_parse(line, &info, 0);

    sal_printf("MSTP Configuration Information:\n");
    sal_printf("------------------------------------------------------------------------\n");
    sal_printf(INTEND_2_STR"%-14s : %s\n", "Format Id", "todo");
    sal_printf(INTEND_2_STR"%-14s : %s\n", "Name", info.value[TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME]);
    sal_printf(INTEND_2_STR"%-14s : %s\n", "Revision Level", info.value[TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL]);
    sal_printf(INTEND_2_STR"%-14s : %s\n", "Digest", info.value[TBL_MSTP_GLOBAL_FLD_MSTP_DIGEST]);
    sal_printf(INTEND_2_STR"%-14s : %s\n", "Instances", "todo");
    sal_printf("------------------------------------------------------------------------\n");

    return PM_E_NONE;
}

static int32
_cdbctl_show_mstp_global_of_stp_vlan(FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];
    uint32 enable = FALSE;
    
    if (NULL == sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        return PM_E_NONE;
    }
    
    cfg_get_parse(line, &info, 0);

    sal_printf("Openflow Spanning-tree VLAN Information:\n");
    
    enable = (info.value[TBL_MSTP_GLOBAL_FLD_ENABLE][0] != '0') ? TRUE : FALSE;
    sal_printf("%-20s : %s\n", "Exclude ALL VLANs", enabled_str(enable_value(info.value[TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN])));
    sal_printf("%-20s : %s\n", "Include VLANs", info.value[TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP]);
    if (!enable)
    {
        sal_printf("Spanning-tree is disabled, the openflow VLAN configuration does not take effect\n");
    }
    
    return PM_E_NONE;
}

static int32
_cdbctl_show_mstp_global_stp(uint32 brief, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    char  brg_glb_line[FILE_READ_LINE_LEN];
    uint32 enable = FALSE;
    uint32 bpdufilter = FALSE;
    uint32 bpduguard = FALSE;
    cfg_parse_get_info_t info;
    cfg_parse_get_info_t brg_glb_info;
    uint32 priority = 0;
    uint32 port_id = 0;
    uint32 tmp = 0;
    uint32 type = 0;
    sal_time_t last_tc;
    char *root_id_str = NULL;
    char *brg_id_str = NULL;
    char *addr_str = NULL;
    char *p = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        return PM_E_NONE;
    }
    
    PM_E_RETURN(cdbclt_get_tbl_line(TBL_BRG_GLOBAL, NULL, 0, brg_glb_line, FILE_READ_LINE_LEN));
    
    cfg_get_parse(line, &info, 0);
    cfg_get_parse(brg_glb_line, &brg_glb_info, 0);
    enable = (info.value[TBL_MSTP_GLOBAL_FLD_ENABLE][0] != '0') ? TRUE : FALSE;
    bpdufilter = (info.value[TBL_MSTP_GLOBAL_FLD_BPDUFILTER][0] != '0') ? TRUE : FALSE;
    bpduguard = (info.value[TBL_MSTP_GLOBAL_FLD_BPDUGUARD][0] != '0') ? TRUE : FALSE;
    tmp = cdb_int_str2val(info.value[TBL_MSTP_GLOBAL_FLD_LAST_TC_TIME], &rc);
    last_tc = (sal_time_t)tmp;
    
    root_id_str = info.value[TBL_MSTP_GLOBAL_FLD_ROOT_ROOTID];
    brg_id_str = info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_ID];
    
    type = cdb_enum_str2val(mstp_type_strs, MSTP_TYPE_MAX, info.value[TBL_MSTP_GLOBAL_FLD_TYPE]);
    if (brief)
    {
        sal_printf("Spanning-tree %s, Mode %s\n", 
            enabled_str(enable), 
            info.value[TBL_MSTP_GLOBAL_FLD_TYPE]);
        
        addr_str = cdbclient_brgid_2_pri_addr(root_id_str);
        priority = cdb_int_str2val(root_id_str, &rc);
        sal_printf("%-11s %-11s %u (0x%04X)\n", "Root ID", "Priority", priority, priority);
        sal_printf("%-11s %-11s %s\n", "", "Address", addr_str);
        sal_printf("%-11s Hello Time %-2s sec  Max Age %-2s sec  Forward Delay %-2s sec\n",
            "",
            info.value[TBL_MSTP_GLOBAL_FLD_ROOT_HELLOTIME],
            info.value[TBL_MSTP_GLOBAL_FLD_ROOT_MAXAGE],
            info.value[TBL_MSTP_GLOBAL_FLD_ROOT_FWDDELAY]);
        sal_printf("\n");

        addr_str = cdbclient_brgid_2_pri_addr(brg_id_str);
        priority = cdb_int_str2val(brg_id_str, &rc);
        sal_printf("%-11s %-11s %u (0x%04X)\n", "Bridge ID", "Priority", priority, priority);
        sal_printf("%-11s %-11s %s\n", "", "Address", addr_str);
        sal_printf("%-11s Hello Time %-2s sec  Max Age %-2s sec  Forward Delay %-2s sec\n",
            "",
            info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME],
            info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE],
            info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY]);
        sal_printf("%-11s Aging Time %-2s sec\n",
            "",
            brg_glb_info.value[TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME]);
        sal_printf("\n");
    }
    else
    {
        sal_printf("-----[Spanning-tree %s][Mode %s]-----\n", 
            enabled_str(enable), 
            info.value[TBL_MSTP_GLOBAL_FLD_TYPE]);

        addr_str = cdbclient_brgid_2_pri_addr(root_id_str);
        priority = cdb_int_str2val(root_id_str, &rc);
        port_id = cdb_int_str2val(info.value[TBL_MSTP_GLOBAL_FLD_ROOT_DESIGNATEDPORTID], &rc);
        sal_printf("%-20s - %04X-%s\n", "Root Id", priority, addr_str);
        addr_str = cdbclient_brgid_2_pri_addr(brg_id_str);
        priority = cdb_int_str2val(brg_id_str, &rc);
        sal_printf("%-20s - %04X-%s\n", "Bridge Id", priority, addr_str);

        if (MSTP_TYPE_MSTP == type)
        {
            sal_printf("%-20s - HelloTime %s, MaxAge %s, ForwardDelay %s, MaxHops %s\n", 
                "Bridge Times",
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME],
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE],
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY],
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS]);
        }
        else
        {
            sal_printf("%-20s - HelloTime %s, MaxAge %s, ForwardDelay %s\n", 
                "Bridge Times",
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME],
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE],
                info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY]);
        }
        
        sal_printf("%-20s - Path Cost %s, Port %u, Bridge Priority %s\n",
            "Root Info",
            info.value[TBL_MSTP_GLOBAL_FLD_ROOT_ROOTPATHCOST],
            MSTP_GET_PORT_FROM_PORTID(port_id),
            info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY]);
        
        sal_printf("%-20s - %s\n", "Path Cost Standard", info.value[TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD]);
        sal_printf("%-20s - %s\n", "Tx Hold Count", info.value[TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT]);
        sal_printf("%-20s - %s\n", "Max Hops", info.value[TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS]);
        sal_printf("%-20s - BPDU-Filter %s, BPDU-Guard %s\n", 
            "Port Default Action",
            enabled_str(bpdufilter),
            enabled_str(bpduguard));

        p = sal_ctime(&last_tc);
        sal_printf("%-20s - %s", "Last TopoChange Time", p);
        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_mstp_global(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *field_str = (char*)p_data;
    uint32 brief = FALSE;
    
    if (NULL == field_str)
    {
        return PM_E_FAIL;
    }

    if (sal_strstr(field_str, "mst"))
    {
        _cdbctl_show_mstp_global_mst(fp, p_info);
    }
    else if (sal_strstr(field_str, "of_stp_vlan"))
    {
        _cdbctl_show_mstp_global_of_stp_vlan(fp, p_info);
    }
    else
    {    
        if (sal_strstr(field_str, "brief"))
        {
            brief = TRUE;
        }
        
        _cdbctl_show_mstp_global_stp(brief, fp, p_info);
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_mstp_instance(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    cfg_parse_get_info_t info;
    char  line[FILE_READ_LINE_LEN];

    sal_printf("%-9s %s\n", "Instance", "Vlans mapped");
    sal_printf("------------------------------------------------------------------------\n");
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        sal_printf("%-9s %s\n", info.value[TBL_MSTP_INSTANCE_FLD_KEY], info.value[TBL_MSTP_INSTANCE_FLD_VLAN]);
    }
    sal_printf("------------------------------------------------------------------------\n");

    return PM_E_NONE;
}

