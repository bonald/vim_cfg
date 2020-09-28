
#include "proto.h"
#include "cdbclient.h"

#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_switch_debug_define.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_app_debug_define.h"
#include "gen/tbl_hsrv_debug_define.h"
#include "gen/tbl_lsrv_debug_define.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_authd_debug_define.h"
#include "gen/tbl_opm_debug_define.h"
#include "gen/tbl_quagga_debug_define.h"
#include "gen/tbl_ssm_debug_define.h"
#include "gen/tbl_ptp_debug_define.h"
#include "gen/tbl_dhcpv6_debug_define.h"
#include "gen/tbl_ntp_cfg_define.h"
#include "gen/tbl_ntp_cfg.h"
#include "gen/tbl_openflow_debug_define.h"
#include "gen/tbl_openflow_debug.h"
#define DBG_PRINT_LINE(_ptr_str, _submodule, _typestr, _type) \
do { \
    if (param.detail || ('0' != info.value[_type][0])) \
    { \
        if (banner) \
        { \
            sal_printf("%-12s %-16s %-20s %-6s\n", "Module", "Feature", "Type", "Status"); \
            sal_printf("------------+----------------+--------------------+------\n"); \
            banner = FALSE; \
        } \
        sal_printf("%-12s %-16s %-20s %-6s\n", _ptr_str, submodule_str[_submodule], _typestr, ON_OFF_FROM_1_0(info.value[_type])); \
        _ptr_str = nul_str; \
    } \
} while (0)

#define DBG_PRINT_LINE_NTP(_ptr_str, _submodule, level, _type) \
do { \
    if (param.detail || ('0' != info.value[_type][0])) \
    { \
        if (banner) \
        { \
            sal_printf("%-12s %-16s %-20s %-6s\n", "Module", "Feature", "Type", "Status"); \
            sal_printf("------------+----------------+--------------------+------\n"); \
            banner = FALSE; \
        } \
        sal_printf("%-12s %-16s %-20s %-6s\n", _ptr_str, submodule_str[_submodule], ntp_level[level], ON_OFF_FROM_1_0(info.value[_type])); \
        _ptr_str = nul_str; \
    } \
} while (0)


typedef struct
{
    uint32 detail;
    char *sub_module;
} debug_parse_param_t;

int32
cdbctl_parse_params(char *para_str, debug_parse_param_t *param)
{
    cfg_parse_info_t parse_info;
    int32 rc = 0;

    param->detail = FALSE;
    param->sub_module = NULL;

    if (NULL == para_str)
    {
        return 0;
    }
    
    rc = cfg_cmd_parse_para(para_str, &parse_info);
    if (rc < 0)
    {
        return rc;
    }

    if (2 == parse_info.argc)
    {
        if (sal_strlen(parse_info.argv[0]))
        {
            param->sub_module = parse_info.argv[0];
        }
        param->detail = TRUE;
    }
    else if (1 == parse_info.argc)
    {
        if (0 == sal_strcmp(parse_info.argv[0], "detail"))
        {
            param->sub_module = NULL;
            param->detail = TRUE;
        }
        else
        {
            if (sal_strlen(parse_info.argv[0]))
            {
                param->sub_module = parse_info.argv[0];
            }
            param->detail = FALSE;
        }
    }

    return 0;
}

int32
cdbctl_show_switch_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "switch";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[SWTHDBG_TYPE_MAX] = {"stp", "mlag", "lacp", "igmp-snooping", "ipsg", "lldp", "rpcapi"};
    
    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_STP]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_STP);
        }
        else if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_MLAG]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_MLAG);
        }
        else if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_LACP]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_LACP);
        }
        else if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_IGSP]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_IGSP);
        }
        else if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_IPSG]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_IPSG);
        }
        else if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_LLDP]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_LLDP);
        }
        else if (!sal_strcmp(para, submodule_str[SWTHDBG_TYPE_RPCAPI]))
        {
            GLB_SET_BIT(type, SWTHDBG_TYPE_RPCAPI);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_STP))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_STP, "event", TBL_SWITCH_DEBUG_FLD_STP_EVENT);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_STP, "packet tx", TBL_SWITCH_DEBUG_FLD_STP_PACKET_TX);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_STP, "packet rx", TBL_SWITCH_DEBUG_FLD_STP_PACKET_RX);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_STP, "proto cist", TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_CIST);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_STP, "proto msti", TBL_SWITCH_DEBUG_FLD_STP_PROTOCOL_MSTI);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_STP, "timer", TBL_SWITCH_DEBUG_FLD_STP_TIMER);
        }
        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_MLAG))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_MLAG, "event", TBL_SWITCH_DEBUG_FLD_MLAG_EVENT);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_MLAG, "packet", TBL_SWITCH_DEBUG_FLD_MLAG_PACKET);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_MLAG, "protocol", TBL_SWITCH_DEBUG_FLD_MLAG_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_MLAG, "timer", TBL_SWITCH_DEBUG_FLD_MLAG_TIMER);
        }
        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_LACP))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LACP, "event", TBL_SWITCH_DEBUG_FLD_LACP_EVENT);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LACP, "packet", TBL_SWITCH_DEBUG_FLD_LACP_PACKET);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LACP, "protocol", TBL_SWITCH_DEBUG_FLD_LACP_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LACP, "timer", TBL_SWITCH_DEBUG_FLD_LACP_TIMER);
        }
        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_IGSP))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IGSP, "event", TBL_SWITCH_DEBUG_FLD_IGSP_EVENT);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IGSP, "packet tx", TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_TX);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IGSP, "packet rx", TBL_SWITCH_DEBUG_FLD_IGSP_PACKET_RX);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IGSP, "protocol", TBL_SWITCH_DEBUG_FLD_IGSP_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IGSP, "timer", TBL_SWITCH_DEBUG_FLD_IGSP_TIMER);
        }
        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_IPSG))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IPSG, "event", TBL_SWITCH_DEBUG_FLD_IPSG_EVENT);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_IPSG, "error", TBL_SWITCH_DEBUG_FLD_IPSG_ERROR);
        }
        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_LLDP))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LLDP, "event", TBL_SWITCH_DEBUG_FLD_LLDP_EVENT);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LLDP, "packet tx", TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_TX);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LLDP, "packet rx", TBL_SWITCH_DEBUG_FLD_LLDP_PACKET_RX);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LLDP, "protocol", TBL_SWITCH_DEBUG_FLD_LLDP_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_LLDP, "timer", TBL_SWITCH_DEBUG_FLD_LLDP_TIMER);
        }
        if (GLB_IS_BIT_SET(type, SWTHDBG_TYPE_RPCAPI))
        {
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_RPCAPI, "request", TBL_SWITCH_DEBUG_FLD_RPCAPI_REQUEST);
            DBG_PRINT_LINE(ptr_str, SWTHDBG_TYPE_RPCAPI, "command", TBL_SWITCH_DEBUG_FLD_RPCAPI_COMMAND);
        }
    }
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_route_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "route";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[RTDBG_TYPE_MAX] = {"route", "arp", "arp-inspection", "vrrp", "ipsla", "track", "nd", "nat"};//Add by zhush for bug 52833,2019-07-24
    
    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_ROUTE]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_ROUTE);
        }
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_ARP]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_ARP);
        }
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_ARPINSPECTION]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_ARPINSPECTION);
        }
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_VRRP]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_VRRP);
        } 
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_IPSLA]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_IPSLA);
        }
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_TRACK]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_TRACK);
        }
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_ND]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_ND);
        }
/*Add by zhush for bug 52833,2019-07-24*/
        else if (!sal_strcmp(para, submodule_str[RTDBG_TYPE_NAT]))
        {
            GLB_SET_BIT(type, RTDBG_TYPE_NAT);
        }
/*end zhush*/
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_ROUTE))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ROUTE, "ipv4", TBL_ROUTE_DEBUG_FLD_ROUTE_IPV4);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ROUTE, "ipv6", TBL_ROUTE_DEBUG_FLD_ROUTE_IPV6);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ROUTE, "netlink", TBL_ROUTE_DEBUG_FLD_ROUTE_NETLINK);
        }
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_ARP))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARP, "event", TBL_ROUTE_DEBUG_FLD_ARP_EVENT);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARP, "packet", TBL_ROUTE_DEBUG_FLD_ARP_PACKET);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARP, "protocol", TBL_ROUTE_DEBUG_FLD_ARP_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARP, "timer", TBL_ROUTE_DEBUG_FLD_ARP_TIMER);
        }
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_ARPINSPECTION))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARPINSPECTION, "event", TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_EVENT);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARPINSPECTION, "packet", TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PACKET);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARPINSPECTION, "protocol", TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ARPINSPECTION, "timer", TBL_ROUTE_DEBUG_FLD_ARPINSPECTION_TIMER);
        }
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_VRRP))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_VRRP, "event", TBL_ROUTE_DEBUG_FLD_VRRP_EVENTS);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_VRRP, "packet_send", TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_SEND);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_VRRP, "packet_recv", TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_RECV);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_VRRP, "packet_detail", TBL_ROUTE_DEBUG_FLD_VRRP_PACKET_DETAIL);
        }
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_IPSLA))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_IPSLA, "event", TBL_ROUTE_DEBUG_FLD_IPSLA_EVENT);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_IPSLA, "packet", TBL_ROUTE_DEBUG_FLD_IPSLA_PACKET);
        }
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_TRACK))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_TRACK, "event", TBL_ROUTE_DEBUG_FLD_TRACK_EVENT);
        }
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_ND))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ND, "event", TBL_ROUTE_DEBUG_FLD_ND_EVENT);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ND, "packet", TBL_ROUTE_DEBUG_FLD_ND_PACKET);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ND, "protocol", TBL_ROUTE_DEBUG_FLD_ND_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_ND, "timer", TBL_ROUTE_DEBUG_FLD_ND_TIMER);
        }
/*Add by zhush for bug 52833,2019-07-24*/
        if (GLB_IS_BIT_SET(type, RTDBG_TYPE_NAT))
        {
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_NAT, "event", TBL_ROUTE_DEBUG_FLD_NAT_EVENT);
            DBG_PRINT_LINE(ptr_str, RTDBG_TYPE_NAT, "timer", TBL_ROUTE_DEBUG_FLD_NAT_TIMER);
        }
/*end zhush*/
    }
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_app_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "app";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[RTDBG_TYPE_MAX] = {"snmp", "rmon", "ntp"};
    char  *ntp_level[] = {"emerg", "alert", "crit", "error", "warning", "notice", "info", "debug"};
    char szBuf[1024];
    uint32 debug_level = 0;
    cdbctl_parse_params(para, &param);

    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[APPDBG_TYPE_SNMP]))
        {
            GLB_SET_BIT(type, APPDBG_TYPE_SNMP);
        }
        else if (!sal_strcmp(para, submodule_str[APPDBG_TYPE_RMON]))
        {
            GLB_SET_BIT(type, APPDBG_TYPE_RMON);
        }
        else if (!sal_strcmp(para, submodule_str[APPDBG_TYPE_NTP]))
        {
            GLB_SET_BIT(type, APPDBG_TYPE_NTP);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        if (GLB_IS_BIT_SET(type, APPDBG_TYPE_SNMP))
        {
            DBG_PRINT_LINE(ptr_str, APPDBG_TYPE_SNMP, "agent", TBL_APP_DEBUG_FLD_SNMP_AGENT);
        }
        if (GLB_IS_BIT_SET(type, APPDBG_TYPE_RMON))
        {
            DBG_PRINT_LINE(ptr_str, APPDBG_TYPE_RMON, "event", TBL_APP_DEBUG_FLD_RMON_EVENT);
            DBG_PRINT_LINE(ptr_str, APPDBG_TYPE_RMON, "alarm", TBL_APP_DEBUG_FLD_RMON_ALARM);
            DBG_PRINT_LINE(ptr_str, APPDBG_TYPE_RMON, "stats", TBL_APP_DEBUG_FLD_RMON_STATS);
            DBG_PRINT_LINE(ptr_str, APPDBG_TYPE_RMON, "history", TBL_APP_DEBUG_FLD_RMON_HISTORY);
        }
        if (GLB_IS_BIT_SET(type, APPDBG_TYPE_NTP))
        {
            sal_cmd_exec(szBuf, 512, "cdbctl read/cdb/app/ntp_cfg/debug_level");
            sal_sscanf(szBuf, "/debug_level=%d", &debug_level);
            DBG_PRINT_LINE_NTP(ptr_str, APPDBG_TYPE_NTP, debug_level, TBL_APP_DEBUG_FLD_NTP_DEBUG_LEVEL);
        }
    }
    
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_ssm_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "ssm";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[SSM_TYPE_MAX] = {"ssm"};

    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[0]))
        {
            GLB_SET_BIT(type, SSM_TYPE_SSM);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (GLB_IS_BIT_SET(type, SSM_TYPE_SSM))
        {
            DBG_PRINT_LINE(ptr_str, SSM_TYPE_SSM, "event",     TBL_SSM_DEBUG_FLD_SSM_EVENT);
            DBG_PRINT_LINE(ptr_str, SSM_TYPE_SSM, "packet_tx", TBL_SSM_DEBUG_FLD_SSM_PACKET_TX);
            DBG_PRINT_LINE(ptr_str, SSM_TYPE_SSM, "packet_rx", TBL_SSM_DEBUG_FLD_SSM_PACKET_RX);
            DBG_PRINT_LINE(ptr_str, SSM_TYPE_SSM, "timer",     TBL_SSM_DEBUG_FLD_SSM_TIMER);
        }
    }

    if (!banner)
    {
        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_openflow_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "openflow";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[OPENFLOWDBG_TYPE_MAX] = {"openflow"};

    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[0]))
        {
            GLB_SET_BIT(type, OPENFLOWDBG_TYPE_OPENFLOW);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (GLB_IS_BIT_SET(type, OPENFLOWDBG_TYPE_OPENFLOW))
        {
            DBG_PRINT_LINE(ptr_str, OPENFLOWDBG_TYPE_OPENFLOW, "vconn", TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_VCONN);
            DBG_PRINT_LINE(ptr_str, OPENFLOWDBG_TYPE_OPENFLOW, "rconn",  TBL_OPENFLOW_DEBUG_FLD_OPENFLOW_RCONN);
        }
    }

    if (!banner)
    {
        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_ptp_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "ptp";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[PTPDBG_TYPE_MAX] = {"ptp"};
    
    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[PTPDBG_TYPE_PTP]))
        {
            GLB_SET_BIT(type, PTPDBG_TYPE_PTP);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        if (GLB_IS_BIT_SET(type, PTPDBG_TYPE_PTP))
        {
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "communicatin", TBL_PTP_DEBUG_FLD_PTP_COMMUNICATION_DEBUG);
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "bmc", TBL_PTP_DEBUG_FLD_PTP_BMC_DEBUG);
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "timer", TBL_PTP_DEBUG_FLD_PTP_TIMER_DEBUG);
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "synchronization", TBL_PTP_DEBUG_FLD_PTP_SYNCHRONIZATION_DEBUG);
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "packet rx", TBL_PTP_DEBUG_FLD_PTP_PACKET_RX_DEBUG);
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "packet tx", TBL_PTP_DEBUG_FLD_PTP_PACKET_TX_DEBUG);
            DBG_PRINT_LINE(ptr_str, PTPDBG_TYPE_PTP, "packet discard", TBL_PTP_DEBUG_FLD_PTP_PACKET_DISCARD_DEBUG);
        }
    }
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_quagga_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "route-proto";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[QUAGGADBG_TYPE_MAX] = {"ospf", "bgp", "igmp", "mroute", "pim"};
    
    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[QUAGGADBG_TYPE_OSPF]))
        {
            GLB_SET_BIT(type, QUAGGADBG_TYPE_OSPF);
        }
        else if (!sal_strcmp(para, submodule_str[QUAGGADBG_TYPE_BGP]))
        {
            GLB_SET_BIT(type, QUAGGADBG_TYPE_BGP);
        }
        else if (!sal_strcmp(para, submodule_str[QUAGGADBG_TYPE_IGMP]))
        {
            GLB_SET_BIT(type, QUAGGADBG_TYPE_IGMP);
        }
        else if (!sal_strcmp(para, submodule_str[QUAGGADBG_TYPE_MROUTE]))
        {
            GLB_SET_BIT(type, QUAGGADBG_TYPE_MROUTE);
        }
        else if (!sal_strcmp(para, submodule_str[QUAGGADBG_TYPE_PIM]))
        {
            GLB_SET_BIT(type, QUAGGADBG_TYPE_PIM);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        /* OSPF */
        if (GLB_IS_BIT_SET(type, QUAGGADBG_TYPE_OSPF))
        {
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "event abr", TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ABR);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "event ia", TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_IA);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "event route", TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_ROUTE);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "event spf", TBL_QUAGGA_DEBUG_FLD_OSPF_EVENT_SPF);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "ism event", TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_EVENTS);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "ism status", TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_STATUS);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "ism timers", TBL_QUAGGA_DEBUG_FLD_OSPF_ISM_TIMERS);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "lsa flooding", TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_FLOODING);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "lsa generate", TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_GENERATE);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "lsa install", TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_INSTALL);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "lsa refresh", TBL_QUAGGA_DEBUG_FLD_OSPF_LSA_REFRESH);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "nsm events", TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_EVENTS);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "nsm status", TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_STATUS);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "nsm timers", TBL_QUAGGA_DEBUG_FLD_OSPF_NSM_TIMERS);
            //DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "nssa", TBL_QUAGGA_DEBUG_FLD_OSPF_NSSA);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "packet dd", TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_DD);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "packet hello", TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_HELLO);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "packet ls-ack", TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_ACK);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "packet ls-request", TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_REQUEST);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "packet ls-update", TBL_QUAGGA_DEBUG_FLD_OSPF_PACKET_LS_UPDATE);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "zebra interface", TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_INTERFACE);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_OSPF, "zebra redistribute", TBL_QUAGGA_DEBUG_FLD_OSPF_ZEBRA_REDISTRIBUTE);
        }
#if 0
        /* BGP */        
        if (GLB_IS_BIT_SET(type, QUAGGADBG_TYPE_BGP))
        {
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "as4 segment", TBL_QUAGGA_DEBUG_FLD_BGP_AS4_SEGMENT);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "event", TBL_QUAGGA_DEBUG_FLD_BGP_EVENT);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "filters", TBL_QUAGGA_DEBUG_FLD_BGP_FILTERS);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "fsm", TBL_QUAGGA_DEBUG_FLD_BGP_FSM);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "keepalives", TBL_QUAGGA_DEBUG_FLD_BGP_KEEPALIVES);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "updates in", TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_IN);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "updates out", TBL_QUAGGA_DEBUG_FLD_BGP_UPDATES_OUT);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_BGP, "zebra", TBL_QUAGGA_DEBUG_FLD_BGP_ZEBRA);
        }
#endif
        /* IGMP */        
        if (GLB_IS_BIT_SET(type, QUAGGADBG_TYPE_IGMP))
        {
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_IGMP, "event", TBL_QUAGGA_DEBUG_FLD_IGMP_EVENT);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_IGMP, "packet", TBL_QUAGGA_DEBUG_FLD_IGMP_PACKET);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_IGMP, "trace", TBL_QUAGGA_DEBUG_FLD_IGMP_TRACE);
        }

        /* MROUTE */        
        if (GLB_IS_BIT_SET(type, QUAGGADBG_TYPE_MROUTE))
        {
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_MROUTE, "on", TBL_QUAGGA_DEBUG_FLD_MROUTE_ON);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_MROUTE, "detail", TBL_QUAGGA_DEBUG_FLD_MROUTE_DETAIL);
        }

        /* PIM */        
        if (GLB_IS_BIT_SET(type, QUAGGADBG_TYPE_PIM))
        {
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_PIM, "event", TBL_QUAGGA_DEBUG_FLD_PIM_EVENT);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_PIM, "packet", TBL_QUAGGA_DEBUG_FLD_PIM_PACKET);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_PIM, "packetdump_receive", TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_RECEIVE);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_PIM, "packetdump_send", TBL_QUAGGA_DEBUG_FLD_PIM_PACKETDUMP_SEND);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_PIM, "trace", TBL_QUAGGA_DEBUG_FLD_PIM_TRACE);
            DBG_PRINT_LINE(ptr_str, QUAGGADBG_TYPE_PIM, "zebra", TBL_QUAGGA_DEBUG_FLD_PIM_ZEBRA);
        }
    }
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_dhcp_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    uint32  banner      = TRUE;
    char    *para       = p_data;
    char    *mod_str    = "dhcp";
    char    *nul_str    = "";
    char    *ptr_str    = mod_str;
    char    *submodule_str[] = {"client", "relay", "snooping"};

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        /* The print is none when related debug is off and not show detail. refer to this func */
        cdbctl_parse_params(para, &param);

        cfg_get_parse(line, &info, 0);
        DBG_PRINT_LINE(ptr_str, 0, "error", TBL_DHCP_DEBUG_FLD_CLIENT_ERROR);
        DBG_PRINT_LINE(ptr_str, 0, "events", TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS);
        DBG_PRINT_LINE(ptr_str, 0, "packet", TBL_DHCP_DEBUG_FLD_CLIENT_PACKET);
        DBG_PRINT_LINE(ptr_str, 0, "dump", TBL_DHCP_DEBUG_FLD_CLIENT_DUMP);
        DBG_PRINT_LINE(ptr_str, 1, "error", TBL_DHCP_DEBUG_FLD_RELAY_ERROR);
        DBG_PRINT_LINE(ptr_str, 1, "events", TBL_DHCP_DEBUG_FLD_RELAY_EVENTS);
        DBG_PRINT_LINE(ptr_str, 1, "packet", TBL_DHCP_DEBUG_FLD_RELAY_PACKET);
        DBG_PRINT_LINE(ptr_str, 1, "dump", TBL_DHCP_DEBUG_FLD_RELAY_DUMP);
        DBG_PRINT_LINE(ptr_str, 2, "error", TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR);
        DBG_PRINT_LINE(ptr_str, 2, "events", TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT);
        DBG_PRINT_LINE(ptr_str, 2, "packet", TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET);
        DBG_PRINT_LINE(ptr_str, 2, "dump", TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP);
    }
    if (!banner)
    {
        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_dhcpv6_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    uint32  banner      = TRUE;
    char    *para       = p_data;
    char    *mod_str    = "dhcpv6";
    char    *nul_str    = "";
    char    *ptr_str    = mod_str;
    char    *submodule_str[] = {"client", "relay", "snooping"};

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        /* The print is none when related debug is off and not show detail. refer to this func */
        cdbctl_parse_params(para, &param);

        cfg_get_parse(line, &info, 0);
        DBG_PRINT_LINE(ptr_str, 0, "error", TBL_DHCPV6_DEBUG_FLD_CLIENTV6_ERROR);
        DBG_PRINT_LINE(ptr_str, 0, "events", TBL_DHCPV6_DEBUG_FLD_CLIENTV6_EVENTS);
        DBG_PRINT_LINE(ptr_str, 0, "packet", TBL_DHCPV6_DEBUG_FLD_CLIENTV6_PACKET);
        DBG_PRINT_LINE(ptr_str, 0, "dump", TBL_DHCPV6_DEBUG_FLD_CLIENTV6_DUMP);
    }
    if (!banner)
    {
        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_opm_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    uint32  banner      = TRUE;
    char    *para       = p_data;
    char    *mod_str    = "opm";
    char    *nul_str    = "";
    char    *ptr_str    = mod_str;
    char    *submodule_str[OPM_TYPE_MAX] = {"erps", "g8131", "tpoam", "g8032", "cfm"};
    uint32  type = DBG_TYPE_ALL;

    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[OPM_TYPE_ERPS]))
        {
            GLB_SET_BIT(type, OPM_TYPE_ERPS);
        }
        else if (!sal_strcmp(para, submodule_str[OPM_TYPE_G8131]))
        {
            GLB_SET_BIT(type, OPM_TYPE_G8131);
        }
        else if (!sal_strcmp(para, submodule_str[OPM_TYPE_TPOAM]))
        {
            GLB_SET_BIT(type, OPM_TYPE_TPOAM);
        }
        else if (!sal_strcmp(para, submodule_str[OPM_TYPE_G8032]))
        {
            GLB_SET_BIT(type, OPM_TYPE_G8032);
        }
        else if (!sal_strcmp(para, submodule_str[OPM_TYPE_CFM]))
        {
            GLB_SET_BIT(type, OPM_TYPE_CFM);
        }
        else 
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (GLB_IS_BIT_SET(type, OPM_TYPE_ERPS))
        {
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_ERPS, "events",    TBL_OPM_DEBUG_FLD_ERPS_EVENT);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_ERPS, "packet",    TBL_OPM_DEBUG_FLD_ERPS_PACKET);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_ERPS, "protocol",  TBL_OPM_DEBUG_FLD_ERPS_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_ERPS, "timer",     TBL_OPM_DEBUG_FLD_ERPS_TIMER);
        }
        if (GLB_IS_BIT_SET(type, OPM_TYPE_G8032))
        {
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8032, "events",    TBL_OPM_DEBUG_FLD_G8032_EVENT);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8032, "packet_tx",    TBL_OPM_DEBUG_FLD_G8032_PACKET_TX);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8032, "packet_rx",  TBL_OPM_DEBUG_FLD_G8032_PACKET_RX);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8032, "timer",     TBL_OPM_DEBUG_FLD_G8032_TIMER);
        }
        #ifdef OFPRODUCT
        if (GLB_IS_BIT_SET(type, OPM_TYPE_G8131))
        {
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8131, "tx",    TBL_OPM_DEBUG_FLD_G8131_TX);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8131, "rx",    TBL_OPM_DEBUG_FLD_G8131_RX);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_G8131, "event",  TBL_OPM_DEBUG_FLD_G8131_EVENT);
        }
        if (GLB_IS_BIT_SET(type, OPM_TYPE_TPOAM))
        {
            //DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "dm 1dm rx", TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMRX_DEBUG);
            //DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "dm 1dm tx", TBL_OPM_DEBUG_FLD_TPOAM_DM_1DMTX_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "dm dmm", TBL_OPM_DEBUG_FLD_TPOAM_DM_DMM_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "dm dmr", TBL_OPM_DEBUG_FLD_TPOAM_DM_DMR_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "dm process", TBL_OPM_DEBUG_FLD_TPOAM_DM_COMM_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "event", TBL_OPM_DEBUG_FLD_TPOAM_EVENT_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "lb lbm", TBL_OPM_DEBUG_FLD_TPOAM_LB_LBM_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "lb lbr", TBL_OPM_DEBUG_FLD_TPOAM_LB_LBR_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "lm", TBL_OPM_DEBUG_FLD_TPOAM_LM_COMM_DEBUG);
            //DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "lm lmm", TBL_OPM_DEBUG_FLD_TPOAM_LM_LMM_DEBUG);
            //DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "lm lmr", TBL_OPM_DEBUG_FLD_TPOAM_LM_LMR_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "packet rx", TBL_OPM_DEBUG_FLD_TPOAM_PACKET_RX_DEBUG);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_TPOAM, "packet tx", TBL_OPM_DEBUG_FLD_TPOAM_PACKET_TX_DEBUG);
        }
        #endif
        if (GLB_IS_BIT_SET(type, OPM_TYPE_CFM))
        {
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_CFM, "event",    TBL_OPM_DEBUG_FLD_CFM_EVENT);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_CFM, "packet",   TBL_OPM_DEBUG_FLD_CFM_PACKET);
            DBG_PRINT_LINE(ptr_str, OPM_TYPE_CFM, "timer",     TBL_OPM_DEBUG_FLD_CFM_TIMER);
        }
    }

    if (!banner)
    {
        sal_printf("\n");
    }

    return PM_E_NONE;
}

int32
cdbctl_show_lsrv_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "lsrv";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[HSRVDBG_TYPE_MAX] = 
        {"card", "port", "fiber", "fan", "psu", "led", "thermal", "synce"};

    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        if (!sal_strcmp(para, submodule_str[LSRVDBG_TYPE_CARD]))
        {
            GLB_SET_BIT(type, LSRVDBG_TYPE_CARD);
        }
        else if (!sal_strcmp(para, submodule_str[LSRVDBG_TYPE_SYNCE]))
        {
            GLB_SET_BIT(type, LSRVDBG_TYPE_SYNCE);\
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);
        if (GLB_IS_BIT_SET(type, LSRVDBG_TYPE_CARD))
        {
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "card", TBL_LSRV_DEBUG_FLD_CARD);
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "port", TBL_LSRV_DEBUG_FLD_PORT);
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "fiber", TBL_LSRV_DEBUG_FLD_FIBER);
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "fan", TBL_LSRV_DEBUG_FLD_FAN);
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "psu", TBL_LSRV_DEBUG_FLD_PSU);
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "led", TBL_LSRV_DEBUG_FLD_LED);
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "thermal", TBL_LSRV_DEBUG_FLD_THERMAL);
        }
        if (GLB_IS_BIT_SET(type, LSRVDBG_TYPE_CARD))
        {
            DBG_PRINT_LINE(ptr_str, LSRVDBG_TYPE_CARD, "synce", TBL_LSRV_DEBUG_FLD_SYNCE);
        }
    }
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}

int32
cdbctl_show_hsrv_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "fea";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[HSRVDBG_TYPE_MAX] = 
        {"vswitch", "interface", "message", "cpu-packet", "ip", "nexthop", "vlan", "fdb", "l2mc", "ipmc", "agg", "ipuc", "neighbor", "ecmp", "acl", "stats", "qos", "mirror", "stp", "ptp", "tpoam", "l2protocol"};
    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_VSWITCH]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_VSWITCH);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_INTERFACE]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_INTERFACE);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_MESSAGE]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_MESSAGE);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_CPU]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_CPU);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_IP]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_IP);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_NEXTHOP]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_NEXTHOP);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_VLAN]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_VLAN);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_FDB]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_FDB);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_L2MC]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_L2MC);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_AGG]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_AGG);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_IPUC]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_IPUC);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_NEIGHBOR]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_NEIGHBOR);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_ECMP]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_ECMP);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_ACL]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_ACL);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_STATS]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_STATS);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_QOS]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_QOS);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_STP]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_STP);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_PTP]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_PTP);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_TPOAM]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_TPOAM);
        }
        else if (!sal_strcmp(para, submodule_str[HSRVDBG_TYPE_L2PROTOCOL]))
        {
            GLB_SET_BIT(type, HSRVDBG_TYPE_L2PROTOCOL);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }

    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_VSWITCH))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_VSWITCH, "vswitch", TBL_HSRV_DEBUG_FLD_VSWITCH);
        }

        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_INTERFACE))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_INTERFACE, "l2if", TBL_HSRV_DEBUG_FLD_INTERFACE_L2IF);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_INTERFACE, "l3if", TBL_HSRV_DEBUG_FLD_INTERFACE_L3IF);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_INTERFACE, "ifdb", TBL_HSRV_DEBUG_FLD_INTERFACE_IFDB);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_MESSAGE))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_MESSAGE, "pm2hsrv", TBL_HSRV_DEBUG_FLD_MESSAGE_PM2HSRV);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_MESSAGE, "hsrv2pm", TBL_HSRV_DEBUG_FLD_MESSAGE_HSRV2PM);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_CPU))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "direction rx", TBL_HSRV_DEBUG_FLD_CPU_RX);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "direction tx", TBL_HSRV_DEBUG_FLD_CPU_TX);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "raw data", TBL_HSRV_DEBUG_FLD_CPU_RAW);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason arp", TBL_HSRV_DEBUG_FLD_CPU_ARP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason bgp", TBL_HSRV_DEBUG_FLD_CPU_BGP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason bpdu", TBL_HSRV_DEBUG_FLD_CPU_BPDU);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason dhcp", TBL_HSRV_DEBUG_FLD_CPU_DHCP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason eapol", TBL_HSRV_DEBUG_FLD_CPU_EAPOL);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason erps", TBL_HSRV_DEBUG_FLD_CPU_ERPS);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason ipda", TBL_HSRV_DEBUG_FLD_CPU_IPDA);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason igmp", TBL_HSRV_DEBUG_FLD_CPU_IGMP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason l3copy_cpu", TBL_HSRV_DEBUG_FLD_CPU_L3COPYCPU);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason lldp", TBL_HSRV_DEBUG_FLD_CPU_LLDP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason macda", TBL_HSRV_DEBUG_FLD_CPU_MACDA);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason mac_limit", TBL_HSRV_DEBUG_FLD_CPU_MACLIMIT);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason mac_mismatch", TBL_HSRV_DEBUG_FLD_CPU_MACMISMATCH);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason ospf", TBL_HSRV_DEBUG_FLD_CPU_OSPF);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason other", TBL_HSRV_DEBUG_FLD_CPU_OTHER);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason ptp", TBL_HSRV_DEBUG_FLD_CPU_PTP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason rip", TBL_HSRV_DEBUG_FLD_CPU_RIP);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_CPU, "reason slow_proto", TBL_HSRV_DEBUG_FLD_CPU_SLOWPROTO);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_IP))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_IP, "icmp", TBL_HSRV_DEBUG_FLD_IP_ICMP);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_NEXTHOP))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_NEXTHOP, submodule_str[HSRVDBG_TYPE_NEXTHOP], TBL_HSRV_DEBUG_FLD_NEXTHOP);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_VLAN))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_VLAN, submodule_str[HSRVDBG_TYPE_VLAN], TBL_HSRV_DEBUG_FLD_VLAN);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_FDB))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_FDB, submodule_str[HSRVDBG_TYPE_FDB], TBL_HSRV_DEBUG_FLD_FDB);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_L2MC))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_L2MC, submodule_str[HSRVDBG_TYPE_L2MC], TBL_HSRV_DEBUG_FLD_L2MC);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_AGG))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_AGG, submodule_str[HSRVDBG_TYPE_AGG], TBL_HSRV_DEBUG_FLD_AGG);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_IPUC))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_IPUC, submodule_str[HSRVDBG_TYPE_IPUC], TBL_HSRV_DEBUG_FLD_IPUC);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_NEIGHBOR))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_NEIGHBOR, submodule_str[HSRVDBG_TYPE_NEIGHBOR], TBL_HSRV_DEBUG_FLD_NEIGHBOR);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_ECMP))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_ECMP, submodule_str[HSRVDBG_TYPE_ECMP], TBL_HSRV_DEBUG_FLD_ECMP);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_ACL))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_ACL, submodule_str[HSRVDBG_TYPE_ACL], TBL_HSRV_DEBUG_FLD_ACL);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_STATS))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_STATS, submodule_str[HSRVDBG_TYPE_STATS], TBL_HSRV_DEBUG_FLD_STATS);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_QOS))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_QOS, "class", TBL_HSRV_DEBUG_FLD_QOS_CLASS);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_QOS, "policer", TBL_HSRV_DEBUG_FLD_QOS_POLICER);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_QOS, "process", TBL_HSRV_DEBUG_FLD_QOS_PROCESS);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_QOS, "queue", TBL_HSRV_DEBUG_FLD_QOS_QUEUE);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_STP))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_STP, submodule_str[HSRVDBG_TYPE_STP], TBL_HSRV_DEBUG_FLD_STP);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_PTP))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_PTP, "cfg", TBL_HSRV_DEBUG_FLD_PTP_CFG);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_PTP, "interrupt", TBL_HSRV_DEBUG_FLD_PTP_INTERRUPT);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_PTP, "pkt_rx", TBL_HSRV_DEBUG_FLD_PTP_PKT_RX);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_PTP, "pkt_tx", TBL_HSRV_DEBUG_FLD_PTP_PKT_TX);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_TPOAM))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_TPOAM, "common", TBL_HSRV_DEBUG_FLD_TPOAM_COMMON);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_TPOAM, "event", TBL_HSRV_DEBUG_FLD_TPOAM_EVENT);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_TPOAM, "pkt_rx", TBL_HSRV_DEBUG_FLD_TPOAM_PKT_RX);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_TPOAM, "pkt_tx", TBL_HSRV_DEBUG_FLD_TPOAM_PKT_TX);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_TPOAM, "update", TBL_HSRV_DEBUG_FLD_TPOAM_UPDATE);
        }
        if (GLB_IS_BIT_SET(type, HSRVDBG_TYPE_L2PROTOCOL))
        {
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_L2PROTOCOL, "pkt_rx", TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_RX);
            DBG_PRINT_LINE(ptr_str, HSRVDBG_TYPE_L2PROTOCOL, "pkt_tx", TBL_HSRV_DEBUG_FLD_L2PROTOCOL_PKT_TX);
        }
    }
    if (!banner)
    {
        sal_printf("\n");
    }
            
    return PM_E_NONE;
}

int32
cdbctl_show_auth_debug(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    debug_parse_param_t param;
    char  *para = p_data;
    uint32 type = DBG_TYPE_ALL;
    uint32 banner = TRUE;
    char  *mod_str = "auth";
    char  *nul_str = "";
    char  *ptr_str = mod_str;
    char  *submodule_str[RTDBG_TYPE_MAX] = {"aaa"};
    
    cdbctl_parse_params(para, &param);
    if (param.sub_module)
    {
        type = 0;
        if (!sal_strcmp(para, submodule_str[AUDBG_TYPE_FLAGS]))
        {
            GLB_SET_BIT(type, AUDBG_TYPE_FLAGS);
        }
        else
        {
            sal_printf("Invalid type %s\n", para);
            return PM_E_FAIL;
        }
    }
    
    if (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        cfg_get_parse(line, &info, 0);

        if (GLB_IS_BIT_SET(type, AUDBG_TYPE_FLAGS))
        {
            DBG_PRINT_LINE(ptr_str, AUDBG_TYPE_FLAGS, "event", TBL_AUTHD_DEBUG_FLD_AUTH_EVENT);
            DBG_PRINT_LINE(ptr_str, AUDBG_TYPE_FLAGS, "packet", TBL_AUTHD_DEBUG_FLD_AUTH_PACKET);
            DBG_PRINT_LINE(ptr_str, AUDBG_TYPE_FLAGS, "protocol", TBL_AUTHD_DEBUG_FLD_AUTH_PROTOCOL);
            DBG_PRINT_LINE(ptr_str, AUDBG_TYPE_FLAGS, "timer", TBL_AUTHD_DEBUG_FLD_AUTH_TIMER);
        }
    }
    if (!banner)
    {
        sal_printf("\n");
    }
    
    return PM_E_NONE;
}
