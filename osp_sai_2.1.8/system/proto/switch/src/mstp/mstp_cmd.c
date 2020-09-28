#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "switch.h"
#include "switch_api.h"
#include "brg.h"
#include "mstp_define.h"
#include "mstp_api.h"
#include "mstp_sm.h"
#include "mstp.h"

#ifdef CONFIG_MSTP

#define PORT_WIDTH_STR "%-13s"

static char *mstp_version_str[] = 
{
    "STP",
    "Invalid",
    "RSTP",
    "MSTP"
};

static const char *mstp_edgeport_show_strs[MSTP_EDGEPORT_MAX] =
{
    "Disabled",
    "Enabled",
    "Default"
};

const char *mstp_link_strs[MSTP_LINK_MAX] =
{
    "shared",
    "p2p",
    "auto"
};

const char *mstp_link_show_strs[MSTP_LINK_MAX] =
{
    "Shared",
    "P2p",
    "Auto"
};

uint32 g_mstp_inited = FALSE;

#define MSTP_GET_COND_ALL       0
#define MSTP_GET_COND_RUNNING   1
#define MSTP_GET_COND_ENABLED   2
#define MSTP_GET_COND_DISABLED  3

#define MAX_OF_INCLUDE_VLAN_COUNT      10

int32
_mstp_cmd_show_mstp_cist_global(uint32 is_mst, uint32 is_brief, FILE *fp, cfg_cmd_para_t *para);

static char*
_mstp_brgid_2_format(char *brgid)
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
_mstp_cmd_get_mstp_port(char *ifname, uint32 condtion, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    FILE *fp = NULL;
    tbl_mstp_port_key_t key;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    char ifname_ext[IFNAME_EXT_SIZE];
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == ifname)
    {
        CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
        {
            p_db_if = (tbl_interface_t*)p_db_port->p_if;
            if (MSTP_GET_COND_RUNNING == condtion)
            {
                if (!tbl_interface_is_running(p_db_if))
                {
                    continue;
                }
            }
            else if (MSTP_GET_COND_ENABLED == condtion)
            {
                if (!p_db_port->portEnabled)
                {
                    continue;
                }
            }
            else if (MSTP_GET_COND_DISABLED == condtion)
            {
                if (p_db_port->port_enable)
                {
                    continue;
                }
            }
            tbl_mstp_port_dump_one(p_db_port, &args);
        }
    }
    else
    {
        sal_memset(&key, 0, sizeof(key));
        sal_strncpy(key.name, ifname, IFNAME_SIZE);
        p_db_port = tbl_mstp_port_get_mstp_port(&key);
        if (!p_db_port)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            IFNAME_ETH2FULL(ifname, ifname_ext);
            p_db_if = tbl_interface_get_interface_by_name(ifname);
            if (p_db_if)
            {
                CFG_CONFLICT_RET("The interface %s is not L2 interface", ifname_ext);
            }
            else
            {
                CFG_CONFLICT_RET("The interface %s not exist", ifname_ext);
            }
        }
        tbl_mstp_port_dump_one(p_db_port, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_mstp_cmd_get_mstp_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    FILE *fp = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_mstp_global_dump_one(p_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

#define _MSTP_PORT_CMD_

static int32
_mstp_cmd_set_port_admin_force_version(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (BPDU_VERSION_DEFAULT == p_port->adminForceVersion)
    {
        switch (p_glb->type)
        {
        case MSTP_TYPE_STP:
            p_port->forceVersion = BPDU_VERSION_STP;
            break;
        case MSTP_TYPE_RSTP:
            p_port->forceVersion = BPDU_VERSION_RSTP;
            break;
        case MSTP_TYPE_MSTP:
            p_port->forceVersion = BPDU_VERSION_MSTP;
            break;
        default:
            break;
        }
    }
    else
    {
        if (BPDU_VERSION_STP != p_port->adminForceVersion
         && BPDU_VERSION_RSTP != p_port->adminForceVersion
         && BPDU_VERSION_MSTP != p_port->adminForceVersion)
        {
            CFG_CONFLICT_RET("Version %u not supported", p_port->adminForceVersion);
        }

        switch (p_glb->type)
        {
        case MSTP_TYPE_STP:
            if (p_port->adminForceVersion > BPDU_VERSION_STP)
            {
                CFG_CONFLICT_RET("Protocol %s cannot set version %u", 
                    cdb_enum_val2str(mstp_type_strs, MSTP_TYPE_MAX, p_glb->type), 
                    p_port->adminForceVersion);
            }
            break;
        case MSTP_TYPE_RSTP:
            if (p_port->adminForceVersion > BPDU_VERSION_RSTP)
            {
                CFG_CONFLICT_RET("Protocol %s cannot set version %u", 
                    cdb_enum_val2str(mstp_type_strs, MSTP_TYPE_MAX, p_glb->type), 
                    p_port->adminForceVersion);
            }
            break;
        default:
            break;
        }
    }
    
    /* Added by kcao 2017-01-05 for bug 42411, if same to mstp mode, need uses change to default config */
    switch (p_glb->type)
    {
    case MSTP_TYPE_STP:
        if (BPDU_VERSION_STP == p_port->adminForceVersion)
        {
            p_port->adminForceVersion = BPDU_VERSION_DEFAULT;
        }
        break;
    case MSTP_TYPE_RSTP:
        if (BPDU_VERSION_RSTP == p_port->adminForceVersion)
        {
            p_port->adminForceVersion = BPDU_VERSION_DEFAULT;
        }
        break;
    case MSTP_TYPE_MSTP:
        if (BPDU_VERSION_MSTP == p_port->adminForceVersion)
        {
            p_port->adminForceVersion = BPDU_VERSION_DEFAULT;
        }
        break;
    default:
        break;
    }

    /* 2. check duplicate config */
    if (p_db_port->adminForceVersion == p_port->adminForceVersion)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_admin_force_version(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_enable(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL; 
    int32 rc = PM_E_NONE;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_port->ifindex);

    /* 1. config check */
    if (p_db_if && p_db_if->brgif && p_db_if->brgif->mlag_is_peer_link)
    {
        CFG_CONFLICT_RET("Cannot configure for MLAG peer-link has been configured");
    }
    if (p_db_if->erps_enable)
    {
        CFG_CONFLICT_RET("Cannot configure for ERPS has been enabled");
    }
    
    if (p_db_if->current_g8032_ring_count)
    {
        CFG_CONFLICT_RET("Cannot configure for G8032 has been configured");
    }
    /* 2. check duplicate config */
    if (p_port->port_enable == p_db_port->port_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_enable(p_db_port, p_port->port_enable, FALSE));
    return rc;
}

static int32
_mstp_cmd_set_port_restricted_tcn(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->restricted_tcn == p_db_port->restricted_tcn)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_restricted_tcn(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_restricted_role(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->restricted_role == p_db_port->restricted_role)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_restricted_role(p_port));

    /* modified by liwh for bug 37514, 2016-05-17 */
    mstp_sm_role_selection();
    /* liwh end */
    
    return rc;
}

static int32
_mstp_cmd_set_port_priority(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (0 != (p_port->priority % MSTP_PORT_PRIORITY_GRANULARITY))
    {
        CFG_CONFLICT_RET("Port priority must be in multiples of %u", MSTP_PORT_PRIORITY_GRANULARITY);
    }
    
    /* 2. check duplicate config */
    if (p_port->priority == p_db_port->priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_priority(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_path_cost(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    /* 1. config check, 0 is remove admin PathCost */
    switch (p_mstp_glb->pathcost_standard)
    {
    case MSTP_PATH_COST_1T:
        if (p_port->adminPortPathCost > MSTP_PATH_COST_1T_MAX)
        {
            CFG_CONFLICT_RET("Path cost should in range [1, 200000000]");
        }
        break;

    case MSTP_PATH_COST_1D_1998:
        if (p_port->adminPortPathCost > MSTP_PATH_COST_1D1998_MAX)
        {
            CFG_CONFLICT_RET("Path cost should in range [1, 65535]");
        }
        break;

    default:
        CFG_CONFLICT_RET("Invalid path cost standard");
        break;
    }
    
    /* 2. check duplicate config */
    if (p_port->adminPortPathCost == p_db_port->adminPortPathCost)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_path_cost(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_bpdufilter(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->admin_bpdufilter == p_db_port->admin_bpdufilter)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_bpdufilter(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_bpduguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->admin_bpduguard == p_db_port->admin_bpduguard)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_bpduguard(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_loopguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_db_port->admin_rootguard)
    {
        CFG_CONFLICT_RET("Conflict - Loop-Guard function couldn't be configured on a port which enabled Root-Guard function");    
    }

    if (p_db_port->adminEdge)
    {
        CFG_CONFLICT_RET("Conflict - Loop-Guard function couldn't be configured on a Edge port");    
    }
    /* liwh end */
    
    /* 2. check duplicate config */
    if (p_port->admin_loopguard == p_db_port->admin_loopguard)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_loopguard(p_port, p_db_port));
    
    return rc;
}

static int32
_mstp_cmd_set_port_rootguard(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_db_port->admin_loopguard)
    {
        CFG_CONFLICT_RET("Conflict - Root-Guard function couldn't be configured on a port which enabled Loop-Guard function");    
    }
    /* liwh end */
    
    /* 2. check duplicate config */
    if (p_port->admin_rootguard == p_db_port->admin_rootguard)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_rootguard(p_port, p_db_port));
    
    return rc;
}

static int32
_mstp_cmd_set_port_admin_edge(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    /* modified by liwh for bug 37762, 2016-06-21 */
    if (p_db_port->admin_loopguard)
    {
        CFG_CONFLICT_RET("Conflict - A port which enabled Loop-Guard function couldn't be configured as a Edge port");    
    }
    /* liwh end */
    
    /* 2. check duplicate config */
    if (p_port->adminEdge == p_db_port->adminEdge)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_admin_edge(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_admin_p2pmac(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->adminP2PMac == p_db_port->adminP2PMac)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_port_admin_p2pmac(p_port));
    return rc;
}

static int32
_mstp_cmd_set_port_mcheck(tbl_mstp_port_t *p_port, tbl_mstp_port_t *p_db_port, cfg_cmd_para_t *para)
{
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    ctclib_slistnode_t *listnode = NULL;

    if (p_db_port)
    {
        mstp_api_set_port_mcheck(p_db_port);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
        {
            mstp_api_set_port_mcheck(p_db_port);
        }
    }

    return PM_E_NONE;
}

int32
mstp_cmd_process_mstp_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_mstp_port_t port;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    char *ifname = NULL;
    uint32 parse_field = TRUE;
    uint32 get_condition = MSTP_GET_COND_ALL;
    int32 value = 0;
    int32 i = 0;
    field_parse_t field;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MSTP_PORT);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
        i++;
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        sal_memset(&port, 0, sizeof(port));
        sal_strncpy(port.key.name, argv[0], IFNAME_SIZE);

        /* check exist */
        p_db_port = tbl_mstp_port_get_mstp_port(&port.key);
        if (!p_db_port)
        {
            if (TBL_MSTP_PORT_FLD_MCHECK == field_id)
            {
            }
            else
            {
                IFNAME_ETH2FULL(port.key.name, ifname_ext);
                p_db_if = tbl_interface_get_interface_by_name(port.key.name);
                if (p_db_if)
                {
                    IF_MGR_CHECK_OVERLAY_TL_GROUP(p_db_if);

                    if (GLB_IF_MODE_L3 == p_db_if->mode)
                    {
                        CFG_CONFLICT_RET("The interface %s is not L2 interface", ifname_ext);
                    }
                    else
                    {
                        CFG_CONFLICT_RET("The mstp port %s not exist, maybe configured LAG member or monitor destination", ifname_ext);
                    }

                }
                else
                {
                    CFG_CONFLICT_RET("The interface %s not exist", ifname_ext);
                }
            }
        }

        if (p_db_port)
        {
            port.ifindex = p_db_port->ifindex;
            p_db_if = (tbl_interface_t*)p_db_port->p_if;
            if (!p_db_if)
            {
                CFG_CONFLICT_RET("Interface %s not exist", port.key.name);
            }
            /* LAG member port should not participate in STP */
            IF_MGR_CHECK_LAG_MEMBER(p_db_if);
        }

        switch (field_id)
        {
        case TBL_MSTP_PORT_FLD_ADMINFORCEVERSION:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
            if (2 == argc)
            {
                port.adminForceVersion = BPDU_VERSION_DEFAULT;
            }
            else
            {
                PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
                port.adminForceVersion = value;
            }
            rc = _mstp_cmd_set_port_admin_force_version(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_PORT_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.port_enable = value;
            rc = _mstp_cmd_set_port_enable(&port, p_db_port, para);
            break;
    
        case TBL_MSTP_PORT_FLD_RESTRICTED_TCN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.restricted_tcn = value;
            rc = _mstp_cmd_set_port_restricted_tcn(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_RESTRICTED_ROLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.restricted_role = value;
            rc = _mstp_cmd_set_port_restricted_role(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            port.priority = value;
            rc = _mstp_cmd_set_port_priority(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMINPORTPATHCOST:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            port.adminPortPathCost = value;
            rc = _mstp_cmd_set_port_path_cost(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMIN_BPDUFILTER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            value = cdb_enum_str2val(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, argv[i+1]);
            port.admin_bpdufilter = value;
            rc = _mstp_cmd_set_port_bpdufilter(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMIN_BPDUGUARD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            value = cdb_enum_str2val(mstp_edgeport_strs, MSTP_EDGEPORT_MAX, argv[i+1]);
            port.admin_bpduguard = value;
            rc = _mstp_cmd_set_port_bpduguard(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMIN_LOOPGUARD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.admin_loopguard = value;
            rc = _mstp_cmd_set_port_loopguard(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMIN_ROOTGUARD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.admin_rootguard = value;
            rc = _mstp_cmd_set_port_rootguard(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMINEDGE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.adminEdge = value;
            rc = _mstp_cmd_set_port_admin_edge(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_ADMINP2PMAC:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            value = cdb_enum_str2val(mstp_link_strs, 3, argv[i+1]);
            port.adminP2PMac = value;
            rc = _mstp_cmd_set_port_admin_p2pmac(&port, p_db_port, para);
            break;

        case TBL_MSTP_PORT_FLD_MCHECK:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            port.mcheck = value;
            rc = _mstp_cmd_set_port_mcheck(&port, p_db_port, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            get_condition = MSTP_GET_COND_ALL;
            parse_field = FALSE;
            ifname = NULL;
        }
        else if (1 == argc)
        {
            if (!sal_strcmp(argv[0], "disabled"))
            {
                get_condition = MSTP_GET_COND_DISABLED;
                parse_field = FALSE;
                ifname = NULL;
            }
            else if (!sal_strcmp(argv[0], "brief"))
            {
                get_condition = MSTP_GET_COND_ENABLED;
                parse_field = FALSE;
                ifname = NULL;
            }
            else if (!sal_strcmp(argv[0], "up"))
            {
                get_condition = MSTP_GET_COND_ENABLED;
                parse_field = FALSE;
                ifname = NULL;
            }
            else if (!sal_strcmp(argv[0], "all"))
            {
                get_condition = MSTP_GET_COND_ENABLED;
                parse_field = FALSE;
                ifname = NULL;
            }
            else
            {
                if (sal_strstr(argv[0], GLB_IFNAME_ETH_PREFIX) || sal_strstr(argv[0], GLB_IFNAME_AGG_PREFIX))
                {
                    parse_field = FALSE;
                    ifname = argv[0];
                }
            }
        }
        else if (2 == argc)
        {
            get_condition = MSTP_GET_COND_ALL;
            parse_field = FALSE;
            ifname = argv[1];
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid args");
        }
        
        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field));
        }
        rc = _mstp_cmd_get_mstp_port(ifname, get_condition, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _MSTP_PORT_MSTI_CMD_

static int32
_mstp_cmd_set_msti_port_path_cost(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    int32 rc = PM_E_NONE;

    /* 1. config check, 0 is remove admin PathCost */
    switch (p_mstp_glb->pathcost_standard)
    {
    case MSTP_PATH_COST_1T:
        if (p_port->adminPortPathCost > MSTP_PATH_COST_1T_MAX)
        {
            CFG_CONFLICT_RET("Path cost should in range [1, 200000000]");
        }
        break;

    case MSTP_PATH_COST_1D_1998:
        if (p_port->adminPortPathCost > MSTP_PATH_COST_1D1998_MAX)
        {
            CFG_CONFLICT_RET("Path cost should in range [1, 65535]");
        }
        break;

    default:
        CFG_CONFLICT_RET("Invalid path cost standard");
        break;
    }
    
    /* 2. check duplicate config */
    if (p_port->adminPortPathCost == p_db_port->adminPortPathCost)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_msti_port_path_cost(p_port, p_db_port));
    return rc;
}

static int32
_mstp_cmd_set_msti_port_priority(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (0 != (p_port->priority % MSTP_PORT_PRIORITY_GRANULARITY))
    {
        CFG_CONFLICT_RET("Port priority must be in multiples of %u", MSTP_PORT_PRIORITY_GRANULARITY);
    }
    
    /* 2. check duplicate config */
    if (p_port->priority == p_db_port->priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_msti_port_priority(p_port, p_db_port));
    return rc;
}

static int32
_mstp_cmd_set_msti_port_restricted_tcn(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->restricted_tcn == p_db_port->restricted_tcn)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_msti_port_restricted_tcn(p_port));
    return rc;
}

static int32
_mstp_cmd_set_msti_port_restricted_role(tbl_msti_port_t *p_port, tbl_msti_port_t *p_db_port, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_port->restricted_role == p_db_port->restricted_role)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    PM_E_RETURN(mstp_api_set_msti_port_restricted_role(p_port));

    /* modified by liwh for bug 37514, 2016-05-17 */
    mstp_api_reselect_msti_port(p_db_port);
    /* liwh end */
    
    return rc;
}

static int32
_mstp_cmd_get_msti_port(char *ifname, uint32 instance, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_msti_port_master_t *p_master = tbl_msti_port_get_master();
    FILE *fp = NULL;   
    tbl_msti_port_t *p_db_msti = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    CTCLIB_SLIST_LOOP(p_master->port_msti_list, p_db_msti, listnode)
    {
        if (NULL == ifname)
        {
            tbl_msti_port_dump_one(p_db_msti, &args);
        }
        else
        {
            if (0 == sal_strcmp(ifname, p_db_msti->key.name))
            {
                if ((GLB_STP_INSTANCE_MAX == instance) || (p_db_msti->key.instance == instance))
                {
                    tbl_msti_port_dump_one(p_db_msti, &args);
                }
            }
        }
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
mstp_cmd_process_msti_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_msti_port_t msti_port;
    tbl_msti_port_t *p_db_msti_port = NULL;
    cdb_node_t* p_node = NULL;
    char *ifname = NULL;
    char *p = NULL;
    uint32 parse_field = TRUE;
    uint32 instance = GLB_STP_INSTANCE_MAX;
    field_parse_t field;
    uint32 value = 0;
    char ifname_ext[IFNAME_EXT_SIZE];    
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MSTP_PORT);

    sal_memset(&msti_port, 0, sizeof(msti_port));
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(value, argv[0], para->p_rs);
        sal_strncpy(msti_port.key.name, argv[1], IFNAME_SIZE);
        msti_port.key.instance = value;
        /* check exist */
        p_db_msti_port = tbl_msti_port_get_msti_port(&msti_port.key);
        if (NULL == p_db_msti_port)
        {
            IFNAME_ETH2FULL(msti_port.key.name, ifname_ext);
            CFG_CONFLICT_RET("Spanning-tree port %s does not in instance %u", ifname_ext, msti_port.key.instance);
        }
        PM_CFG_GET_UINT(value, argv[3], para->p_rs);
        if (0 == sal_strcmp("path-cost", argv[2]))
        {
            msti_port.adminPortPathCost = value;
            rc = _mstp_cmd_set_msti_port_path_cost(&msti_port, p_db_msti_port, para);
        }
        else if (0 == sal_strcmp("port-priority", argv[2]))
        {
            msti_port.priority = value;
            rc = _mstp_cmd_set_msti_port_priority(&msti_port, p_db_msti_port, para);
        }
        else if (0 == sal_strcmp("restricted-role", argv[2]))
        {
            msti_port.restricted_role = value;
            rc = _mstp_cmd_set_msti_port_restricted_role(&msti_port, p_db_msti_port, para);
        }
        else if (0 == sal_strcmp("restricted-tcn", argv[2]))
        {
            msti_port.restricted_tcn = value;
            rc = _mstp_cmd_set_msti_port_restricted_tcn(&msti_port, p_db_msti_port, para);
        }
        break;
        
    case CDB_OPER_GET:
        if (0 == argc)
        {
            parse_field = FALSE;
            ifname = NULL;
        }
        else if (1 == argc)
        {
            parse_field = FALSE;
            ifname = argv[0];
            p = sal_strchr(argv[0], CMD_KEY_DIV_CHAR);
            if (p)
            {
                p[0] = CMD_ZERO_CHAR;
                p++;
                PM_CFG_GET_UINT(instance, p, para->p_rs);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid args");
        }

        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field));
        }
        rc = _mstp_cmd_get_msti_port(ifname, instance, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _MSTP_INSTANCE_CMD_

static int32
_mstp_cmd_add_instance_vlan(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_t *p_db_inst, cfg_cmd_para_t *para)
{
    vlan_bmp_t tmp_bmp;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;
    
    GLB_BMP_ITER_BEGIN(p_inst->vlan, vid)
    {
        vlan_key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            CFG_CONFLICT_RET("VLAN %u not exist", vid);
        }
    }
    GLB_BMP_ITER_END(p_inst->vlan, vid);

    sal_memcpy(tmp_bmp, p_db_inst->vlan, sizeof(vlan_bmp_t));
    GLB_BMP_ITER_BEGIN(p_inst->vlan, vid)
    {
        if (GLB_BMP_ISSET(p_db_inst->vlan, vid))
        {
        }
        else
        {
            GLB_BMP_SET(tmp_bmp, vid);
            mstp_api_instance_change_to_vlan(vid, p_db_inst);
        }
    }
    GLB_BMP_ITER_END(p_inst->vlan, vid);
    sal_memcpy(p_inst->vlan, tmp_bmp, sizeof(vlan_bmp_t));

    /* 2. call API */
    PM_E_RETURN(mstp_api_instance_set_vlan_db(p_inst));

    mstp_api_msti_reselect(p_db_inst);

    return PM_E_NONE;
}

static int32
_mstp_cmd_add_instance(tbl_mstp_instance_t *p_inst, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_master_t *p_inst_master = tbl_mstp_instance_get_master();
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    /* 1. config check */

    if (p_inst_master->inst_list->count >= GLB_STP_INSTANCE_MAX)
    {
        CFG_CONFLICT_RET("Reach max number of instance %u", GLB_STP_INSTANCE_MAX);
    }

    GLB_BMP_ITER_BEGIN(p_inst->vlan, vid)
    {
        vlan_key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            CFG_CONFLICT_RET("VLAN %u not exist", vid);
        }
    }
    GLB_BMP_ITER_END(p_inst->vlan, vid);

    p_db_inst = mstp_api_add_msti(p_inst);
    if (NULL == p_db_inst)
    {
        CFG_CONFLICT_RET("Create instance %u fail", p_inst->key.instance);
    }
    PM_E_RETURN(_mstp_cmd_add_instance_vlan(p_inst, p_db_inst, para));

    return PM_E_NONE;
}

static int32
_mstp_cmd_del_instance_vlan(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_t *p_db_inst, cfg_cmd_para_t *para)
{
    vlan_bmp_t tmp_bmp;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    vid_t vid = 0;
    uint32 has_vlan = FALSE;
    int32 rc = PM_E_NONE;

    GLB_BMP_ITER_BEGIN(p_inst->vlan, vid)
    {
        vlan_key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            GLB_BMP_UNSET(p_inst->vlan, vid);
        }
    }
    GLB_BMP_ITER_END(p_inst->vlan, vid);

    sal_memcpy(tmp_bmp, p_db_inst->vlan, sizeof(vlan_bmp_t));
    GLB_BMP_ITER_BEGIN(p_inst->vlan, vid)
    {
        if (GLB_BMP_ISSET(p_db_inst->vlan, vid))
        {
            GLB_BMP_UNSET(tmp_bmp, vid);
            mstp_api_instance_change_to_default_vlan(vid, p_db_inst);
        }
        else
        {
        }
    }
    GLB_BMP_ITER_END(p_inst->vlan, vid);
    sal_memcpy(p_inst->vlan, tmp_bmp, sizeof(vlan_bmp_t));

    /* 2. call API */
    PM_E_RETURN(mstp_api_instance_set_vlan_db(p_inst));

    mstp_api_msti_reselect(p_db_inst);

    /* change vlan will maybe also result in delete instance, take too much process */
    /* 3. check need remove instance with no VLANs */
    if (MSTP_DEFAULT_INSTANCE != p_db_inst->key.instance)
    {
        GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
        {
            has_vlan = TRUE;
        }
        GLB_BMP_ITER_END(p_db_inst->vlan, vid);
        if (!has_vlan)
        {
            PM_E_RETURN(mstp_api_del_msti(p_inst));
        }
    }

    return PM_E_NONE;
}

static int32
_mstp_cmd_set_instance_priority(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_t *p_db_inst, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (0 != (p_inst->bridge_priority % MSTP_BRIDGE_PRIORITY_GRANULARITY))
    {
        CFG_CONFLICT_RET("Bridge priority must be in multiples of %u", MSTP_BRIDGE_PRIORITY_GRANULARITY);
    }
    
    if (p_inst->bridge_priority == p_db_inst->bridge_priority)
    {
        return PM_E_NONE;
    }

    rc = mstp_api_set_msti_priority(p_db_inst, p_inst->bridge_priority);

    return rc;
}

static int32
_mstp_cmd_del_instance(tbl_mstp_instance_t *p_inst, tbl_mstp_instance_t *p_db_inst, cfg_cmd_para_t *para)
{
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    /* 2. call API */
    GLB_BMP_ITER_BEGIN(p_db_inst->vlan, vid)
    {
        mstp_api_instance_change_to_default_vlan(vid, p_db_inst);
    }
    GLB_BMP_ITER_END(p_db_inst->vlan, vid);

    sal_memset(&p_inst->vlan, 0, sizeof(vlan_bmp_t));
    PM_E_RETURN(mstp_api_instance_set_vlan_db(p_inst));
    PM_E_RETURN(mstp_api_del_msti(p_inst));
    
    return PM_E_NONE;
}

static int32
_mstp_cmd_get_mstp_instance(tbl_mstp_instance_t *p_db_inst, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_master_t *p_master = tbl_mstp_instance_get_master();
    field_parse_t field;
    FILE *fp = NULL;
    tbl_mstp_instance_t *p_iter_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field));
    args.argv[0] = &field;
    args.argv[1] = fp;
    
    if (NULL == p_db_inst)
    {
        CTCLIB_SLIST_LOOP(p_master->inst_list, p_iter_inst, listnode)
        {
            tbl_mstp_instance_dump_one(p_iter_inst, &args);
        }
    }
    else
    {
        tbl_mstp_instance_dump_one(p_db_inst, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
mstp_cmd_process_mstp_instance(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_mstp_global = tbl_mstp_global_get_mstp_global();
    tbl_mstp_instance_t inst;
    tbl_mstp_instance_t *p_db_inst = NULL;
    int32 value = 0;
    int32 i = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (FALSE == g_mstp_inited)
        {
            /* init when create first instance 0 */
            mstp_init();
            g_mstp_inited = TRUE;
        }
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
        PM_CFG_GET_INTEGER(value, argv[i], para->p_rs);
        sal_memset(&inst, 0, sizeof(inst));
        inst.key.instance = value;

        if (GLB_DEFAULT_INSTANCE != inst.key.instance)
        {
            if (MSTP_TYPE_MSTP != p_mstp_global->type)
            {
/* modified by liwh for bug 54358, 2019-11-13 */
//#ifndef OFPRODUCT
/* liwh end */
                CFG_CONFLICT_RET("Bridge is not configured as MSTP");
/* modified by liwh for bug 54358, 2019-11-13 */
//#endif
/* liwh end */
            }
        }
        
        if (2 == argc)
        {
            rc = cdb_bitmap_str2val(argv[i+1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, inst.vlan, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = rc;
                return rc;
            }
        }
            
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst.key);
        if (NULL == p_db_inst)
        {
            _mstp_cmd_add_instance(&inst, para);
        }
        else
        {
            _mstp_cmd_add_instance_vlan(&inst, p_db_inst, para);
        }
        break;

    case CDB_OPER_DEL:
        if (MSTP_TYPE_MSTP != p_mstp_global->type)
        {
            CFG_CONFLICT_RET("Bridge is not configured as MSTP");
        }
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
        PM_CFG_GET_INTEGER(value, argv[i], para->p_rs);
        sal_memset(&inst, 0, sizeof(inst));
        inst.key.instance = value;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst.key);
        if (NULL == p_db_inst)
        {
            CFG_CONFLICT_RET("Instance %u not exist", inst.key.instance);
        }
        
        if (p_db_inst->g8032_ring_id)
        {
            CFG_CONFLICT_RET("Instance %u has been bound to G8032 ring %d", p_db_inst->key.instance, p_db_inst->g8032_ring_id);
        }
        
        if (argc == 1)
        {
            _mstp_cmd_del_instance(&inst, p_db_inst, para);
        }
        else
        {
            rc = cdb_bitmap_str2val(argv[i+1], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, inst.vlan, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = rc;
                return rc;
            }
            _mstp_cmd_del_instance_vlan(&inst, p_db_inst, para);
        }
        break;

    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_INTEGER(value, argv[i], para->p_rs);
        inst.key.instance = value;
        PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);        
        inst.bridge_priority = value;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst.key);
        if (NULL == p_db_inst)
        {
            CFG_CONFLICT_RET("Instance %u not exist", inst.key.instance);
        }        
        _mstp_cmd_set_instance_priority(&inst, p_db_inst, para);
        break;
            
    case CDB_OPER_GET:
        if (argc == 1)
        {
            PM_CFG_GET_INTEGER(value, argv[i], para->p_rs);
            inst.key.instance = value;
            p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst.key);
            p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst.key);
            if (NULL == p_db_inst)
            {
                CFG_CONFLICT_RET("Instance %u not exist", inst.key.instance);
            }
        }
        rc = _mstp_cmd_get_mstp_instance(p_db_inst, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

#define _MSTP_GLOBAL_CMD_

static int32
_mstp_cmd_set_glb_tc_protection(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->tc_protection == p_db_glb->tc_protection)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_tc_protection(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_tx_hold_count(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->txHoldCount == p_db_glb->txHoldCount)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_tx_hold_count(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_priority(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (0 != (p_glb->bridge_priority % MSTP_BRIDGE_PRIORITY_GRANULARITY))
    {
        CFG_CONFLICT_RET("Bridge priority must be in multiples of %u", MSTP_BRIDGE_PRIORITY_GRANULARITY);
    }
    
    /* 2. check duplicate config */
    if (p_glb->bridge_priority == p_db_glb->bridge_priority)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_priority(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_pathcost_standard(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->pathcost_standard == p_db_glb->pathcost_standard)
    {
        return PM_E_CFG_DUPLICATED;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_pathcost_standard(p_glb));
    return PM_E_NONE;
}

static int32
_mstp_check_fwdDelay_maxAge(uint32 fwdDelay, uint32 maxAge, cfg_cmd_para_t *para)
{
    if ((2 * (fwdDelay - 1)) >= maxAge)
    {
        return PM_E_NONE;
    }
    else
    {
        CFG_CONFLICT_RET("Violates ForwardDelay - MaxAge relationship 2*(%s - 1) >= %s", 
            "fwdDelay", "maxAge");
    }
}

static int32
_mstp_check_maxAge_helloTime(uint32 maxAge, uint32 helloTime, cfg_cmd_para_t *para)
{
    if (maxAge >= 2 * (helloTime + 1))
    {
        return PM_E_NONE;
    }
    else
    {
        CFG_CONFLICT_RET("Violates HelloTime - MaxAge relationship 2*(%s + 1) <= %s", 
            "helloTime", "maxAge");
    }
}

static int32
_mstp_cmd_set_glb_fwd_delay(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    PM_E_RETURN(_mstp_check_fwdDelay_maxAge(p_glb->bridge_fwdDelay, p_db_glb->bridge_maxAge, para));
    
    /* 2. check duplicate config */
    if (p_glb->bridge_fwdDelay == p_db_glb->bridge_fwdDelay)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_fwd_delay(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_hello_time(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    PM_E_RETURN(_mstp_check_maxAge_helloTime(p_db_glb->bridge_maxAge, p_glb->bridge_helloTime, para));
    
    /* 2. check duplicate config */
    if (p_glb->bridge_helloTime == p_db_glb->bridge_helloTime)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_hello_time(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_max_age(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    PM_E_RETURN(_mstp_check_maxAge_helloTime(p_glb->bridge_maxAge, p_db_glb->bridge_helloTime, para));
    PM_E_RETURN(_mstp_check_fwdDelay_maxAge(p_db_glb->bridge_fwdDelay, p_glb->bridge_maxAge, para));
    
    /* 2. check duplicate config */
    if (p_glb->bridge_maxAge == p_db_glb->bridge_maxAge)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_max_age(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_max_hops(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->bridge_maxHops == p_db_glb->bridge_maxHops)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_max_hops(p_glb));
    return rc;
}

uint32 mstp_check_g8032_instance(void)
{
    tbl_g8032_ring_master_t *p_g8032_master = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    ctclib_slistnode_t *listnode = NULL;

        

    p_g8032_master = tbl_g8032_ring_get_master();

    if (!p_g8032_master)
    {
        return 0;
    }
    
    CTCLIB_SLIST_LOOP(p_g8032_master->g8032_ring_list, p_g8032_ring, listnode)
    {
        if (p_g8032_ring->instance_count)
        {
            return p_g8032_ring->instance_count;
        }
    }

    return 0;
}

static int32
_mstp_cmd_set_no_mst_config(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (mstp_check_g8032_instance())
    {
        CFG_CONFLICT_RET("Can not delete mst configuration for mstp instances have been bound to g8032");
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_clear_mst_config());
    return rc;
}

static int32
_mstp_cmd_set_glb_type(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    /* 1. config check */

    /* 2. check duplicate config */
    if (p_glb->type == p_db_glb->type)
    {
        return PM_E_NONE;
    }

/*Add by yangl for g8032 2019-08-08*/
#ifdef OFPRODUCT
    if (2 == p_glb->type && p_db_glb->enable)
    {
        CFG_CONFLICT_RET("Can not set mstp mode for spanning tree is enabled");
    }
#endif

    if (mstp_check_g8032_instance())
    {
        CFG_CONFLICT_RET("Can not change stp mode for mstp instances have been bound to g8032");
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_type(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_bpduguard(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->bpduguard == p_db_glb->bpduguard)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_bpduguard(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_bpdufilter(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->bpdufilter == p_db_glb->bpdufilter)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_bpdufilter(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_enable(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_glb->enable == p_db_glb->enable)
    {
        return PM_E_NONE;
    }
#ifdef OFPRODUCT
    if (MSTP_TYPE_MSTP == p_db_glb->type)
    {
        CFG_CONFLICT_RET("Can not enable spanning tree for mstp does not support on this product");
    }
#endif
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_enable(p_glb, FALSE));
    return rc;
}

static int32
_mstp_check_region_name(const char *name, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) > max_len)
    {
        sal_sprintf(err_desc, "String length should not exceed %u", max_len);
        return CDB_NAME_ERR_EXCEED_LEN;
    }

    /* check the chars */
    p = (char *)(name);
    while ('\0' != *p)
    {
        if (('%' != *p))
        {
            p++;
            continue;
        }
        
        sal_sprintf(err_desc, "Character should not has \'%%\'");
        return CDB_NAME_ERR_INVALID_CHAR;
    }

    return CDB_NAME_ERR_NONE;
}

static int32
_mstp_cmd_set_glb_region_name(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (MSTP_TYPE_MSTP != p_db_glb->type)
    {
        CFG_CONFLICT_RET("Bridge is not configured as MSTP");
    }
    
    if (sal_strlen(p_glb->mstp_region_name))
    {
        rc = _mstp_check_region_name(p_glb->mstp_region_name, MSTP_REGION_NAME_LEN, para->p_rs->desc);
        if (rc < 0)
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
    }
    
    /* 2. check duplicate config */
    if (0 == sal_memcmp(p_glb->mstp_region_name, p_db_glb->mstp_region_name, sizeof(p_glb->mstp_region_name)))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_region_name(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_glb_revision_level(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    if (MSTP_TYPE_MSTP != p_db_glb->type)
    {
        CFG_CONFLICT_RET("Bridge is not configured as MSTP");
    }

    if (p_glb->mstp_revision_level > MSTP_REVISION_LEVEL_MAX)
    {
        CFG_CONFLICT_RET("Revision level should be [0, 65535]");
    }
    
    /* 2. check duplicate config */
    if (p_glb->mstp_revision_level == p_db_glb->mstp_revision_level)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_revision_level(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_of_exclude_all_vlan(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 2. check duplicate config */
    if (p_glb->of_exclude_all_vlan == p_db_glb->of_exclude_all_vlan)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(mstp_api_set_glb_of_exclude_all_vlan(p_glb));
    return rc;
}

static int32
_mstp_cmd_set_of_include_vlan(tbl_mstp_global_t *p_glb, tbl_mstp_global_t *p_db_glb, uint32 add, cfg_cmd_para_t *para)
{
    vlan_bmp_t of_include_vlan_bmp;
    uint32 vlan_count = 0;
    tbl_vlan_key_t key;
    tbl_vlan_t *p_vlan = NULL;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    GLB_BMP_ITER_BEGIN(p_glb->of_include_vlan_bmp, vid)
    {
        key.vid = vid;
        p_vlan = tbl_vlan_get_vlan(&key);
        if (NULL == p_vlan)
        {
            CFG_CONFLICT_RET("VLAN %u not exist", vid);
        }
    }
    GLB_BMP_ITER_END(p_glb->of_include_vlan_bmp, vid);
    
    /* 2. check duplicate config */
    if (add)
    {
        sal_memcpy(of_include_vlan_bmp, p_db_glb->of_include_vlan_bmp, sizeof(vlan_bmp_t));
        GLB_BMP_ITER_BEGIN(p_glb->of_include_vlan_bmp, vid)
        {
            GLB_BMP_SET(of_include_vlan_bmp, vid);
        }
        GLB_BMP_ITER_END(p_glb->of_include_vlan_bmp, vid);

        GLB_BMP_ITER_BEGIN(of_include_vlan_bmp, vid)
        {
            vlan_count++;
        }
        GLB_BMP_ITER_END(of_include_vlan_bmp, vid);

        /* bug54106 remove Spanning-tree include VLAN count limitation */
        /* if (vlan_count > MAX_OF_INCLUDE_VLAN_COUNT)
        {
            CFG_CONFLICT_RET("Spanning-tree include VLANs exceed max count %u", MAX_OF_INCLUDE_VLAN_COUNT);
        } */
    }

    GLB_BMP_ITER_BEGIN(p_glb->of_include_vlan_bmp, vid)
    {
        if (add)
        {
            GLB_BMP_SET(p_db_glb->of_include_vlan_bmp, vid);
        }
        else
        {
            GLB_BMP_UNSET(p_db_glb->of_include_vlan_bmp, vid);
        }
    }
    GLB_BMP_ITER_END(p_glb->of_include_vlan_bmp, vid);
    
    /* 1. update local CDB */
    PM_E_RETURN(tbl_mstp_global_set_mstp_global_field(p_db_glb, TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP));

    return rc;
}

int32
mstp_cmd_process_mstp_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_mstp_global_t glb;
    tbl_mstp_global_t *p_db_glb = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 value = 0;
    int32 i = 0;
    field_parse_t field;
    uint32 parse_field = TRUE;
    int32 rc = PM_E_NONE;
    /* modified by liwh for bug 36694, 2016-05-10 */
    tbl_interface_key_t key;
    tbl_interface_t *p_db_if = NULL;
    /* liwh */

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MSTP_GLOBAL);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        /* check exist */
        p_db_glb = tbl_mstp_global_get_mstp_global();
        if (!p_db_glb)
        {
            CFG_CONFLICT_RET("MSTP global not exist");
        }

        switch (field_id)
        {
        case TBL_MSTP_GLOBAL_FLD_TC_PROTECTION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.tc_protection = value;
            rc = _mstp_cmd_set_glb_tc_protection(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_TXHOLDCOUNT:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            glb.txHoldCount = value;
            rc = _mstp_cmd_set_glb_tx_hold_count(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_BRIDGE_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            glb.bridge_priority = value;
            rc = _mstp_cmd_set_glb_priority(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_PATHCOST_STANDARD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            value = cdb_enum_str2val(mstp_path_cost_strs, MSTP_PATH_COST_MAX, argv[i+1]);
            glb.pathcost_standard = value;
            rc = _mstp_cmd_set_glb_pathcost_standard(&glb, p_db_glb, para);
            if (rc < 0)
            {
                if (PM_E_CFG_DUPLICATED == rc)
                {
                    rc = PM_E_NONE;
                }
            }
            else if (PM_E_NONE == rc)
            {
                CFG_PROMPT_RET("Cost of every port has been reset and auto-calculation is available");
            }
            break;

        case TBL_MSTP_GLOBAL_FLD_BRIDGE_FWDDELAY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            glb.bridge_fwdDelay = value;
            rc = _mstp_cmd_set_glb_fwd_delay(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_BRIDGE_HELLOTIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            glb.bridge_helloTime = value;
            rc = _mstp_cmd_set_glb_hello_time(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXAGE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            glb.bridge_maxAge = value;
            rc = _mstp_cmd_set_glb_max_age(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_BRIDGE_MAXHOPS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_INTEGER(value, argv[i+1], para->p_rs);
            glb.bridge_maxHops = value;
            rc = _mstp_cmd_set_glb_max_hops(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_NO_MST_CONFIG:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = _mstp_cmd_set_no_mst_config(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_TYPE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            value = cdb_enum_str2val(mstp_type_lower_strs, MSTP_TYPE_MAX, argv[i+1]);
            if (value < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid type parameter %s", argv[i+1]);
            }
            glb.type = value;
            rc = _mstp_cmd_set_glb_type(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_BPDUGUARD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.bpduguard = value;
            rc = _mstp_cmd_set_glb_bpduguard(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_BPDUFILTER:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.bpdufilter = value;
            rc = _mstp_cmd_set_glb_bpdufilter(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            glb.enable = value;
            rc = _mstp_cmd_set_glb_enable(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_MSTP_REGION_NAME:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            sal_memset(glb.mstp_region_name, 0, sizeof(glb.mstp_region_name));
            if (2 == argc)
            {
                sal_strncpy(glb.mstp_region_name, argv[i+1], sizeof(glb.mstp_region_name));
            }
            rc = _mstp_cmd_set_glb_region_name(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_MSTP_REVISION_LEVEL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);
            glb.mstp_revision_level = value;
            rc = _mstp_cmd_set_glb_revision_level(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
            glb.of_exclude_all_vlan = value;
            rc = _mstp_cmd_set_of_exclude_all_vlan(&glb, p_db_glb, para);
            break;

        case TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            sal_memset(glb.of_include_vlan_bmp, 0, sizeof(glb.of_include_vlan_bmp));
            if (0 == sal_strcmp(argv[1], "add"))
            {
                value = TRUE;
            }
            else
            {
                value = FALSE;
            }
            rc = cdb_bitmap_str2val(argv[2], GLB_VLAN_DEFAULT_VID, GLB_VLAN_MAX, glb.of_include_vlan_bmp, GLB_VLAN_BMP_WORD_MAX, para->p_rs->desc);
            if (rc < 0)
            {
                para->p_rs->rc = rc;
                return rc;
            }
            glb.of_exclude_all_vlan = value;
            rc = _mstp_cmd_set_of_include_vlan(&glb, p_db_glb, value, para);
            break;
        
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            parse_field = FALSE;
        }
        else if (1 == argc)
        {
            if (!sal_strcmp(argv[0], "brief"))
            {
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "up"))
            {
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "all"))
            {
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "mst"))
            {
                parse_field = FALSE;
            }
            else if (!sal_strcmp(argv[0], "of_stp_vlan"))
            {
                parse_field = FALSE;
            }
        }
        else if (2 == argc)
        {
            if (!sal_strcmp(argv[0], "mst"))
            {
                parse_field = FALSE;
            }
            /* modified by liwh for bug 36694, 2016-05-10 */
            else 
            {
                if (!sal_strcmp(argv[0], "brief"))
                {
                    parse_field = FALSE;
                }
                else if (!sal_strcmp(argv[0], "up"))
                {
                    parse_field = FALSE;
                }
                else if (!sal_strcmp(argv[0], "all"))
                {
                    parse_field = FALSE;
                }
                else if (!sal_strcmp(argv[0], "mst"))
                {
                    parse_field = FALSE;
                }
                else if (!sal_strcmp(argv[0], "of_stp_vlan"))
                {
                    parse_field = FALSE;
                }
        
                sal_memset(&key, 0, sizeof(key));
                sal_strncpy(key.name, argv[1], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&key);
                if (!p_db_if)
                {
                    return PM_E_NONE;
                }
            }
            /* liwh end */
        }
        
        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field));
        }
        rc = _mstp_cmd_get_mstp_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
mstp_cmd_process_mstp_clear_stp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_mstp_port_key_t port_key;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];     
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
        if (0 == sal_strcmp(argv[0], "disabled-port"))
        {
            mstp_api_clear_all_disabled_port();
        }
        else if (0 == sal_strcmp(argv[0], "counters"))
        {
            if (argc == 2)
            {
                sal_memset(&port_key, 0, sizeof(port_key));
                sal_strncpy(port_key.name, argv[1], IFNAME_SIZE);
                p_db_port = tbl_mstp_port_get_mstp_port(&port_key);
                if (NULL == p_db_port)
                {
                    IFNAME_ETH2FULL(port_key.name, ifname_ext);
                    p_db_if = tbl_interface_get_interface_by_name(port_key.name);
                    if (p_db_if)
                    {
                        CFG_CONFLICT_RET("The interface %s is not L2 interface", ifname_ext);
                    }
                    else
                    {
                        CFG_CONFLICT_RET("The interface %s not exist", ifname_ext);
                    }
                }
            }
            mstp_api_clear_counters(p_db_port);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid args %s", argv[0]);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

int32
ccs_cfg_mstp_build_one_instance(FILE* fp, cfg_cmd_para_t *para, tbl_mstp_instance_t *p_db_inst)
{
    char str[CMD_BUF_256];
    char tmp[CMD_BUF_32];
    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    uint32 used = 0;
    uint32 tmp_len = 0;
    uint32 line_len = 60;
    
    sal_memset(str, 0, sizeof(str));
    str[0] = CMD_ZERO_CHAR;

    for (offset = 0; offset < GLB_MAX_VLAN_ID; offset++)
    {
        if (GLB_BMP_ISSET(p_db_inst->vlan, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(p_db_inst->vlan, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= line_len)
                {
                    if ((used > 0) && (str[used-1] == ','))
                    {
                        str[used-1] = 0;
                    }
                    sal_fprintf(fp, INTEND_1_STR"instance %u vlan %s\n", p_db_inst->key.instance, str);
                    str[0] = CMD_ZERO_CHAR;
                    used = 0;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 5);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= line_len)
                {
                    if ((used > 0) && (str[used-1] == ','))
                    {
                        str[used-1] = 0;
                    }
                    sal_fprintf(fp, INTEND_1_STR"instance %u vlan %s\n", p_db_inst->key.instance, str);
                    str[0] = CMD_ZERO_CHAR;
                    used = 0;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 10);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
        sal_fprintf(fp, INTEND_1_STR"instance %u vlan %s\n", p_db_inst->key.instance, str);
        str[0] = CMD_ZERO_CHAR;
        used = 0;
    }
    
    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_instance_vlan_mapping(FILE *fp, tbl_mstp_instance_t *p_db_inst, char *next_indent)
{
    char str[CMD_BUF_256];
    char tmp[CMD_BUF_32];
    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    uint32 used = 0;
    uint32 tmp_len = 0;
    uint32 line_len = MAX_CMD_LINE_LEN - sal_strlen(next_indent);
    char *indent = "";
    
    sal_memset(str, 0, sizeof(str));
    str[0] = CMD_ZERO_CHAR;

    for (offset = 0; offset < GLB_MAX_VLAN_ID; offset++)
    {
        if (GLB_BMP_ISSET(p_db_inst->vlan, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(p_db_inst->vlan, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= line_len)
                {
                    if ((used > 0) && (str[used-1] == ','))
                    {
                        str[used-1] = 0;
                    }
                    sal_fprintf(fp, "%s%s\n", indent, str);
                    indent = next_indent;
                    str[0] = CMD_ZERO_CHAR;
                    used = 0;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 5);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= line_len)
                {
                    if ((used > 0) && (str[used-1] == ','))
                    {
                        str[used-1] = 0;
                    }
                    sal_fprintf(fp, "%s%s\n", indent, str);
                    indent = next_indent;
                    str[0] = CMD_ZERO_CHAR;
                    used = 0;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 10);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
        sal_fprintf(fp, "%s%s\n", indent, str);
        indent = next_indent;
        str[0] = CMD_ZERO_CHAR;
        used = 0;
    }
    

    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_mst(cfg_cmd_para_t *para)
{
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_mstp_instance_master_t *p_master = tbl_mstp_instance_get_master();
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _mstp_cmd_show_mstp_cist_global(TRUE, FALSE, fp, para);

    sal_fprintf(fp, "%-9s %s\n", "Instance", "Vlans mapped");
    sal_fprintf(fp, "---------+--------------------------------------------------------------\n");
    CTCLIB_SLIST_LOOP(p_master->inst_list, p_db_inst, listnode)
    {
        sal_fprintf(fp, "%-9u ", p_db_inst->key.instance);
        mstp_cmd_show_mstp_instance_vlan_mapping(fp, p_db_inst, "          ");
    }
    sal_fprintf(fp, "---------+--------------------------------------------------------------\n");
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_config(cfg_cmd_para_t *para)
{
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_mstp_instance_master_t *p_master = tbl_mstp_instance_get_master();
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    FILE *fp = NULL;
    uint32 i = 0;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "MSTP Configuration Information:\n");
    sal_fprintf(fp, "------------------------------------------------------------------------\n");
    sal_fprintf(fp, "  Format Id      : %u\n", p_db_glb->mstp_format_id);
    sal_fprintf(fp, "  Name           : %s\n", p_db_glb->mstp_region_name);
    sal_fprintf(fp, "  Revision Level : %u\n", p_db_glb->mstp_revision_level);
    sal_fprintf(fp, "  Digest         : 0x");
    for (i = 0; i < MSTP_DIGEST_LEN ; i++)
    {
        sal_fprintf(fp, "%02X", p_db_glb->mstp_digest.data[i]);
    }
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "  Instances configured %d\n", (p_master->inst_list->count - 1));
    sal_fprintf(fp, "------------------------------------------------------------------------\n");
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "%-9s %s\n", "Instance", "Vlans mapped");
    sal_fprintf(fp, "---------+--------------------------------------------------------------\n");
    CTCLIB_SLIST_LOOP(p_master->inst_list, p_db_inst, listnode)
    {
        sal_fprintf(fp, "%-9u ", p_db_inst->key.instance);
        mstp_cmd_show_mstp_instance_vlan_mapping(fp, p_db_inst, "          ");
    }
    sal_fprintf(fp, "---------+--------------------------------------------------------------\n");
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
_mstp_cmd_show_mstp_cist_port(uint32 is_brief, tbl_interface_t* p_db_if, tbl_mstp_port_t *p_db_port, uint32 *banner, FILE *fp, cfg_cmd_para_t *para)
{
    tbl_mstp_global_t *p_global = tbl_mstp_global_get_mstp_global();
    uint32 is_stp = FALSE;
    const char *port_state = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    char root_id_str[CMD_BUF_256];
    char regional_root_id_str[CMD_BUF_256];
    char bridge_id_str[CMD_BUF_256];

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    is_stp = (MSTP_TYPE_STP == p_global->type) ? TRUE : FALSE;
    port_state = cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_db_port->pst_state);
    if (p_db_port->oper_rootguard && ((PST_DISCARDING == p_db_port->pst_state) || (PST_LISTENING == p_db_port->pst_state)))
    {
        port_state = "Root-inconsistent";
    }
    /* modified by liwh for bug 37762, 2016-06-21 */
    else if (p_db_port->oper_loopguard && (PST_DISCARDING == p_db_port->pst_state) && (ROLE_DESIGNATED == p_db_port->role))
    {
        port_state = "Loop-inconsistent";    
    }
    else if (is_stp && (PST_DISCARDING == p_db_port->pst_state))
    {
        port_state = "Blocked";
    }

    if (is_brief)
    {
        if (!p_db_port->port_enable)
        {
            sal_fprintf(fp, "%-12s Port protocol disabled\n", ifname_ext);
            return PM_E_NONE;
        }
        if (*banner)
        {
#ifdef HAVE_DEBIAN
            sal_fprintf(fp, "%-12s %-12s %-17s %-10s %9s.%-8s%-6s\n", "Interface", "Role", "State", "Cost", "Priority", "Number", "Type");
            sal_fprintf(fp, "------------+------------+-----------------+-------------------+--------+------\n");
#else
            sal_fprintf(fp, "%-12s %-12s %-17s %-10s %9s.%-8s %-6s\n", "Interface", "Role", "State", "Cost", "Priority", "Number", "Type");
            sal_fprintf(fp, "------------+------------+-----------------+--------------------+--------+------\n");
#endif
            *banner = FALSE;
        }
        
#ifdef HAVE_DEBIAN
        sal_fprintf(fp, "%-12s %-12s %-17s %-10u %9u.%-8u%-6s\n", 
            ifname_ext,
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_db_port->role),
            port_state,
            p_db_port->portPathCost,
            p_db_port->priority,
            p_db_port->ifindex,
            p_db_port->operP2PMac? "P2p" : "Shared");
#else
        sal_fprintf(fp, "%-12s %-12s %-17s %-10u %9u.%-8u %-6s\n", 
            ifname_ext,
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_db_port->role),
            port_state,
            p_db_port->portPathCost,
            p_db_port->priority,
            p_db_port->ifindex,
            p_db_port->operP2PMac? "P2p" : "Shared");
#endif
    }
    else
    {
        if (!p_db_port->port_enable)
        {
            sal_fprintf(fp, " %s: Port protocol disabled", ifname_ext);
            return PM_E_NONE;
        }

        sal_fprintf(fp, "-----[Interface %s][Role %s][State %s]-----\n", 
            ifname_ext,
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_db_port->role),
            port_state);

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Index %u, Priority %u, Id %X\n",
            ifname_ext,
            "Port",
            p_db_port->ifindex,
            p_db_port->priority,
            p_db_port->portId);

        if (p_db_port->adminPortPathCost)
        {
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %u, oper %u \n",
                ifname_ext,
                "Port PathCost",
                p_db_port->adminPortPathCost,
                p_db_port->portPathCost);
        }
        else
        {
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin None, oper %u \n",
                ifname_ext,
                "Port PathCost",
                p_db_port->portPathCost);
        }

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - HelloTime %u, MaxAge %u, ForwardDelay %u, MessageAge %u\n",
            ifname_ext,
            "Port Time",
            p_db_port->port_helloTime,
            p_db_port->port_maxAge,
            p_db_port->port_fwdDelay,
            p_db_port->port_messageAge);

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Hello %d, ForwardDelay %d, MessageAge %d, TopoChange %d\n",
            ifname_ext,
            "Port Timer",
            p_db_port->helloWhen_timer ? ctc_timer_get_remain_sec(p_db_port->helloWhen_timer) : 0,
            p_db_port->fdWhile_timer ? ctc_timer_get_remain_sec(p_db_port->fdWhile_timer) : 0,
            p_db_port->rcvdInfoWhile_timer ? ctc_timer_get_remain_sec(p_db_port->rcvdInfoWhile_timer) : 0,
            p_db_port->tcWhile_timer ? ctc_timer_get_remain_sec(p_db_port->tcWhile_timer) : 0);

        cdb_mstp_brg_id_val2str(root_id_str, CMD_BUF_256, &p_db_port->designated_rootId);
        cdb_mstp_brg_id_val2str(bridge_id_str, CMD_BUF_256, &p_db_port->designated_designatedBridgeId);
        _mstp_brgid_2_format(root_id_str);
        _mstp_brgid_2_format(bridge_id_str);

        sal_memset(regional_root_id_str, 0, sizeof(regional_root_id_str));

        if (MSTP_TYPE_MSTP == p_global->type)
        {
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - CIST %s, Regional %s\n",
                ifname_ext,
                "Root",
                root_id_str,
                regional_root_id_str);

            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Bridge %s\n",
                ifname_ext,
                "Designated",
                bridge_id_str);
            
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Port Id %X, Priority %u\n",
                ifname_ext,
                "Designated",
                p_db_port->designated_designatedPortId,
                ((p_db_port->designated_designatedPortId & 0xF000) >> 12) * 16);
            
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - External %u, Internal %u\n",
                ifname_ext,
                "Path Cost",
                p_db_port->designated_rootPathCost,
                p_db_port->designated_intRootPathCost);
            
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - %u\n",
                ifname_ext,
                "Refer count",
                p_db_port->msti_list.obj_list->count + 1);
        }
        else
        {
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Root %s, Bridge %s\n",
                ifname_ext,
                "Designated",
                root_id_str,
                bridge_id_str);
            
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Port Id %X, Priority %u, Path Cost %u\n",
                ifname_ext,
                "Designated",
                p_db_port->designated_designatedPortId,
                ((p_db_port->designated_designatedPortId & 0xF000) >> 12) * 16,
                p_db_port->designated_rootPathCost);
        }

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Send %u, Received %u\n",
            ifname_ext,
            "BPDU Count",
            p_db_port->tx_bpdu_count,
            p_db_port->rx_bpdu_count);

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - %s, Send %s, Received %s\n",
            ifname_ext,
            "BPDU Version",
            mstp_version_str[p_db_port->forceVersion],
            p_db_port->sendRstp ? ((p_db_port->forceVersion == BPDU_VERSION_MSTP) ? "MSTP" : "RSTP") : "STP",
            p_db_port->rcvdRSTP ? (p_db_port->rcvdMSTP ? "MSTP" : "RSTP") : p_db_port->rcvdSTP ? "STP" : "None");

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
            ifname_ext,
            "Edgeport",
            enabled_str(p_db_port->adminEdge),
            on_off_str(p_db_port->operEdge));

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
            ifname_ext,
            "BPDU Filter",
            mstp_edgeport_show_strs[p_db_port->admin_bpdufilter],
            on_off_str(p_db_port->oper_bpdufilter));

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
            ifname_ext,
            "BPDU Guard",
            mstp_edgeport_show_strs[p_db_port->admin_bpduguard],
            on_off_str(p_db_port->oper_bpduguard));

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
            ifname_ext,
            "Root Guard",
            enabled_str(p_db_port->admin_rootguard),
            on_off_str(p_db_port->oper_rootguard));

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
            ifname_ext,
            "Loop Guard",
            enabled_str(p_db_port->admin_loopguard),
            on_off_str(p_db_port->oper_loopguard));

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %s, oper %s\n",
            ifname_ext,
            "Link Type",
            mstp_link_show_strs[p_db_port->adminP2PMac],
            p_db_port->operP2PMac ? "P2p" : "Shared");

        sal_fprintf(fp, " %s: Forward-transitions %u\n",
            ifname_ext,
            p_db_port->forward_transitions);

        sal_fprintf(fp, "\n");
    }

    return PM_E_NONE;
}

int32
_mstp_cmd_show_mstp_cist_global(uint32 is_mst, uint32 is_brief, FILE *fp, cfg_cmd_para_t *para)
{
    tbl_brg_global_t *p_db_brg_glb = tbl_brg_global_get_brg_global();
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    char buf[MAX_CMD_STR_LEN];
    sal_systime_t current_tv;
    sal_time_t tp;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;

    /* 1. show default instance */
    inst_key.instance = MSTP_DEFAULT_INSTANCE;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    sal_fprintf(fp, "-----[Spanning-tree %s][Mode %s]-----\n", 
        enabled_str(p_db_glb->enable), 
        cdb_enum_val2str(mstp_type_strs, MSTP_TYPE_MAX, p_db_glb->type));

    if (is_mst)
    {
        cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_db_inst->vlan, sizeof(p_db_inst->vlan)/4);
        sal_fprintf(fp, "##### MST%u   : %s\n", p_db_inst->key.instance, buf);
    }
    
    if (is_brief)
    {
        sal_fprintf(fp, "%-11s %-11s %u (0x%04X)\n", "Root ID", "Priority", p_db_glb->root_rootId.priority, p_db_glb->root_rootId.priority);
        sal_fprintf(fp, "%-11s %-11s %s\n", "", "Address", cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_glb->root_rootId.address));
        sal_fprintf(fp, "%-11s Hello Time %-2u sec  Max Age %-2u sec  Forward Delay %-2u sec\n",
            "",
            p_db_glb->root_helloTime,
            p_db_glb->root_maxAge,
            p_db_glb->root_fwdDelay);
        sal_fprintf(fp, "\n");

        if (MSTP_TYPE_MSTP == p_db_glb->type)
        {
            sal_fprintf(fp, "%-11s %-11s %u (0x%04X)\n", "Reg Root ID", "Priority", p_db_glb->root_regionRootId.priority, p_db_glb->root_regionRootId.priority);
            sal_fprintf(fp, "%-11s %-11s %s\n", "", "Address", cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_glb->root_regionRootId.address));
            sal_fprintf(fp, "\n");
        }
        
        sal_fprintf(fp, "%-11s %-11s %u (0x%04X)\n", "Bridge ID", "Priority", p_db_glb->bridge_id.priority, p_db_glb->bridge_id.priority);
        sal_fprintf(fp, "%-11s %-11s %s\n", "", "Address", cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_glb->bridge_id.address));
        sal_fprintf(fp, "%-11s Hello Time %-2u sec  Max Age %-2u sec  Forward Delay %-2u sec\n",
            "",
            p_db_glb->bridge_helloTime,
            p_db_glb->bridge_maxAge,
            p_db_glb->bridge_fwdDelay);
        sal_fprintf(fp, "%-11s Aging Time %-2u sec\n",
            "",
            p_db_brg_glb->oper_fdb_aging_time);
    }
    else
    {
        sal_fprintf(fp, "%-20s - %04x-%s\n", "Root Id", p_db_glb->root_rootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_glb->root_rootId.address));
        if (MSTP_TYPE_MSTP == p_db_glb->type)
        {
            sal_fprintf(fp, "%-20s - %04x-%s\n", "Reg Root Id", p_db_glb->root_regionRootId.priority, 
                cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_glb->root_regionRootId.address));
        }
        sal_fprintf(fp, "%-20s - %04x-%s\n", "Bridge Id", p_db_glb->bridge_id.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_glb->bridge_id.address));

        if (MSTP_TYPE_MSTP == p_db_glb->type)
        {
            sal_fprintf(fp, "%-20s - HelloTime %u, MaxAge %u, ForwardDelay %u, MaxHops %u\n", 
                "Bridge Times",
                p_db_glb->bridge_helloTime,
                p_db_glb->bridge_maxAge,
                p_db_glb->bridge_fwdDelay,
                p_db_glb->bridge_maxHops);
        }
        else
        {
            sal_fprintf(fp, "%-20s - HelloTime %u, MaxAge %u, ForwardDelay %u\n", 
                "Bridge Times",
                p_db_glb->bridge_helloTime,
                p_db_glb->bridge_maxAge,
                p_db_glb->bridge_fwdDelay);
        }
        
        sal_fprintf(fp, "%-20s - Path Cost %u, Port %u, Bridge Priority %u\n",
            "Root Info",
            p_db_glb->root_rootPathCost,
            MSTP_GET_PORT_FROM_PORTID(p_db_glb->root_portId),
            p_db_glb->bridge_priority);

        if (MSTP_TYPE_MSTP == p_db_glb->type)
        {
            sal_fprintf(fp, "%-20s - External %u, Internal %u\n",
            "Root Path Cost",
            p_db_glb->root_rootPathCost,
            p_db_glb->root_intRootPathCost);
        }

        sal_fprintf(fp, "%-20s - %s\n", "Path Cost Standard", cdb_enum_val2str(mstp_path_cost_strs, MSTP_PATH_COST_MAX, p_db_glb->pathcost_standard));
        sal_fprintf(fp, "%-20s - %u\n", "Tx Hold Count", p_db_glb->txHoldCount);
        sal_fprintf(fp, "%-20s - BPDU-Filter %s, BPDU-Guard %s\n", 
            "Port Default Action",
            enabled_str(p_db_glb->bpdufilter),
            enabled_str(p_db_glb->bpduguard));

        sal_gettime(&current_tv);
        tp = current_tv.tv_sec + p_db_glb->last_tc_time - ctc_time_boottime_sec(NULL);

        sal_fprintf(fp, "%-20s - %s", "Last TopoChange Time", sal_ctime(&tp));
    }
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
_mstp_cmd_show_mstp_cist(uint32 is_mst, uint32 is_brief, uint32 is_all, tbl_interface_t* p_db_if, FILE *fp, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_db_port = NULL;
    uint32 banner = TRUE;

    _mstp_cmd_show_mstp_cist_global(is_mst, is_brief, fp, para);

    if (p_db_if)
    {
        p_db_port = p_db_if->mstp_port;
        if (p_db_port)
        {
            _mstp_cmd_show_mstp_cist_port(is_brief, p_db_if, p_db_port, &banner, fp, para);
        }
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            p_db_port = p_db_if->mstp_port;
            if (NULL == p_db_port)
            {
                continue;
            }
            if (!is_all)
            {
                if (!p_db_port->portEnabled)
                {
                    continue;
                }
            }
            if (p_db_port)
            {
                _mstp_cmd_show_mstp_cist_port(is_brief, p_db_if, p_db_port, &banner, fp, para);
            }
        }
    }
    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}

char*
mstp_brgid_2_format(char *brgid)
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

int32
_mstp_cmd_show_mstp_msti_port(uint32 is_brief, tbl_mstp_port_t *p_mstp_port, tbl_msti_port_t *p_db_port, uint32 *banner, FILE *fp, cfg_cmd_para_t *para)
{
    char designated_root_str[CMD_BUF_64];
    char designated_brg_str[CMD_BUF_64];
    char ifname_ext[IFNAME_EXT_SIZE];
    const char *port_state = NULL;

    IFNAME_ETH2FULL(p_mstp_port->key.name, ifname_ext);

    if (!p_mstp_port->port_enable)
    {
        sal_fprintf(fp, " %s: Port protocol disabled", ifname_ext); 
    }
                
    port_state = cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_db_port->pst_state);
    if (p_db_port->oper_rootguard && ((PST_DISCARDING == p_db_port->pst_state) || (PST_LISTENING == p_db_port->pst_state)))
    {
        port_state = "Root-inconsistent";
    }
    /* modified by liwh for bug 37762, 2016-06-21 */
    else if (p_db_port->oper_loopguard && (PST_DISCARDING == p_db_port->pst_state) && (ROLE_DESIGNATED == p_db_port->role))
    {
        port_state = "Loop-inconsistent";    
    }

    if (is_brief)
    {
        if (*banner)
        {
            sal_fprintf(fp, "%-12s %-12s %-17s %-10s %9s.%-8s %-6s\n", "Interface", "Role", "State", "Cost", "Priority", "Number", "Type");
            sal_fprintf(fp, "------------+------------+-----------------+--------------------+--------+------\n");
            *banner = FALSE;
        }


        sal_fprintf(fp, "%-12s %-12s %-17s %-10u %9u.%-8u %-6s\n", 
            ifname_ext,
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_db_port->role),
            port_state,
            p_db_port->portPathCost,
            p_db_port->priority,
            p_mstp_port->ifindex,
            p_mstp_port->operP2PMac? "P2p" : "Shared");
    }
    else
    {
        cdb_mstp_brg_id_val2str(designated_root_str, CMD_BUF_64, &p_db_port->designated_regionRootId);
        cdb_mstp_brg_id_val2str(designated_brg_str, CMD_BUF_64, &p_db_port->designated_designatedBridgeId);
        mstp_brgid_2_format(designated_root_str);
        mstp_brgid_2_format(designated_brg_str);

        sal_fprintf(fp, "-----[Interface %s][Role %s][State %s]-----\n", 
            ifname_ext, 
            cdb_enum_val2str(mstp_port_role_strs, MSTP_PORT_ROLE_MAX, p_db_port->role), 
            port_state);

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Index %u, Priority %u, Id %X\n",
            ifname_ext,
            "Port",
            p_mstp_port->ifindex,
            p_mstp_port->priority,
            p_mstp_port->portId);
        
        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Root %s, Bridge %s\n",
            ifname_ext,
            "Designated",
            designated_root_str,
            designated_brg_str);
        
        if (p_db_port->adminPortPathCost)
        {
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin %u, oper %u \n",
                ifname_ext,
                "Port PathCost",
                p_db_port->adminPortPathCost,
                p_db_port->portPathCost);
        }
        else
        {
            sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - admin None, oper %u \n",
                ifname_ext,
                "Port PathCost",
                p_db_port->portPathCost);
        }
 
        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - CST External %u, MSTI Internal %u\n",
            ifname_ext,
            "Path Cost",
            p_mstp_port->designated_rootPathCost,
            p_db_port->designated_intRootPathCost);
        
        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - CST %u, MSTI %u\n",
            ifname_ext,
            "Priority",
            p_mstp_port->priority,
            p_db_port->priority);
 
        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - HelloTime %u, MaxAge %u, ForwardDelay %u, MessageAge %u\n",
            ifname_ext,
            "Port Time",
            p_db_port->port_helloTime,
            p_db_port->port_maxAge,
            p_db_port->port_fwdDelay,
            p_db_port->port_messageAge);

        sal_fprintf(fp, " %s: "PORT_WIDTH_STR" - Hello %d, ForwardDelay %d, MessageAge %d, TopoChange %d\n",
            ifname_ext,
            "Port Timer",
            p_mstp_port->helloWhen_timer ? ctc_timer_get_remain_sec(p_mstp_port->helloWhen_timer) : 0,
            p_db_port->fdWhile_timer ? ctc_timer_get_remain_sec(p_db_port->fdWhile_timer) : 0,
            p_db_port->rcvdInfoWhile_timer ? ctc_timer_get_remain_sec(p_db_port->rcvdInfoWhile_timer) : 0,
            p_db_port->tcWhile_timer ? ctc_timer_get_remain_sec(p_db_port->tcWhile_timer) : 0);

        sal_fprintf(fp, "\n");
    }
    
    return PM_E_NONE;
}

int32
_mstp_cmd_show_mstp_msti(uint32 is_brief, tbl_mstp_instance_t *p_db_inst, tbl_interface_t* p_db_if, FILE *fp, cfg_cmd_para_t *para)
{
    tbl_msti_port_key_t port_key;
    tbl_msti_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;
    uint32 banner = TRUE;
    char buf[MAX_CMD_STR_LEN];
    
    cdb_bitmap_val2str(buf, MAX_CMD_STR_LEN, p_db_inst->vlan, sizeof(p_db_inst->vlan)/4);
    sal_fprintf(fp, "##### MST%u   : %s\n", p_db_inst->key.instance, buf);

    if (is_brief)
    {
        sal_fprintf(fp, "%-11s %-11s %u (0x%04X)\n", "Root ID", "Priority", p_db_inst->root_rootId.priority, p_db_inst->root_rootId.priority);
        sal_fprintf(fp, "%-11s %-11s %s\n", "", "Address", cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_inst->root_rootId.address));
        sal_fprintf(fp, "%-11s %-11s %u (0x%04X)\n", "Bridge ID", "Priority", p_db_inst->bridge_id.priority, p_db_inst->bridge_id.priority);
        sal_fprintf(fp, "%-11s %-11s %s\n", "", "Address", cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_inst->bridge_id.address));
    }
    else
    {
        sal_fprintf(fp, "%-20s - %04x-%s\n", "MSTI Root Id", p_db_inst->root_rootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_inst->root_rootId.address));
        sal_fprintf(fp, "%-20s - %04x-%s\n", "MSTI Bridge Id", p_db_inst->bridge_id.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_inst->bridge_id.address));
        sal_fprintf(fp, "%-20s - %u\n", "MSTI Bridge Priority", p_db_inst->bridge_priority);
        sal_fprintf(fp, "%-20s - %u\n", "MSTI Root Path Cost", p_db_inst->root_rootPathCost);
        sal_fprintf(fp, "%-20s - %u\n", "MSTI Root Port", p_db_inst->root_portId);
        sal_fprintf(fp, "\n");
    }

    if (p_db_if)
    {
        sal_memset(&port_key, 0, sizeof(port_key));
        sal_strcpy(port_key.name, p_db_if->key.name);
        port_key.instance = p_db_inst->key.instance;
        p_mstp_port = (tbl_mstp_port_t*)p_db_if->mstp_port;
        p_db_port = tbl_msti_port_get_msti_port(&port_key);
        if (p_mstp_port && p_db_port)
        {
            _mstp_cmd_show_mstp_msti_port(is_brief, p_mstp_port, p_db_port, &banner, fp, para);
        }
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_db_inst->port_list.obj_list, p_db_port, listnode)
        {
            p_mstp_port = (tbl_mstp_port_t*)p_db_port->p_port;
            if (NULL == p_mstp_port)
            {
                continue;
            }
            if (!p_mstp_port->portEnabled)
            {
                continue;
            }
            _mstp_cmd_show_mstp_msti_port(is_brief, p_mstp_port, p_db_port, &banner, fp, para);
        }
        sal_fprintf(fp, "\n");
    }
    
    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_brief_detail_interface(tbl_interface_t* p_db_if, uint32 is_mst, uint32 is_brief, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    FILE *fp = NULL;

    p_mstp_port = p_db_if->mstp_port;
    if (NULL == p_mstp_port)
    {
        CFG_INVALID_PARAM_RET("Interface is not MSTP port");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _mstp_cmd_show_mstp_cist(is_mst, is_brief, FALSE, p_db_if, fp, para);

    if (is_mst)
    {
        if (p_mstp_port->port_enable)
        {
            CTCLIB_SLIST_LOOP(p_mstp_port->msti_list.obj_list, p_db_port, listnode)
            {
                inst_key.instance = p_db_port->key.instance;
                p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
                if (p_db_inst)
                {
                    _mstp_cmd_show_mstp_msti(is_brief, p_db_inst, p_db_if, fp, para);
                }
            }
        }
    }
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_brief_detail_instance(tbl_mstp_instance_t* p_db_inst, uint32 is_mst, uint32 is_brief, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _mstp_cmd_show_mstp_msti(is_brief, p_db_inst, NULL, fp, para);
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}
 
int32
mstp_cmd_show_mstp_brief_detail(uint32 is_mst, uint32 is_brief, uint32 is_all, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_master_t *p_master = tbl_mstp_instance_get_master();
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _mstp_cmd_show_mstp_cist(is_mst, is_brief, is_all, NULL, fp, para);
    
    if (is_mst)
    {
        CTCLIB_SLIST_LOOP(p_master->inst_list, p_db_inst, listnode)
        {
            if (MSTP_DEFAULT_INSTANCE == p_db_inst->key.instance)
            {
                continue;
            }

            _mstp_cmd_show_mstp_msti(is_brief, p_db_inst, NULL, fp, para);
        }
    }
    
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_interface(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_msti_port_t *p_db_port = NULL;
    FILE *fp = NULL;
    char ifname_ext[IFNAME_SIZE];

    p_mstp_port = p_db_if->mstp_port;
    if (NULL == p_mstp_port)
    {
        CFG_INVALID_PARAM_RET("Interface is not MSTP port");
    }

    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    _mstp_cmd_show_mstp_cist_global(TRUE, FALSE, fp, para);

    sal_fprintf(fp, "%-9s %-9s %s\n", "Instance", "Interface", "Vlans mapped");
    sal_fprintf(fp, "---------+---------+----------------------------------------------------\n");

    inst_key.instance = GLB_DEFAULT_INSTANCE;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (p_db_inst)
    {
        sal_fprintf(fp, "%-9u %-9s ", p_db_inst->key.instance, "");
        mstp_cmd_show_mstp_instance_vlan_mapping(fp, p_db_inst, "                    ");
    }
        
    CTCLIB_SLIST_LOOP(p_mstp_port->msti_list.obj_list, p_db_port, listnode)
    {
        inst_key.instance = p_db_port->key.instance;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
        if (p_db_inst)
        {
            sal_fprintf(fp, "%-9u %-9s ", p_db_inst->key.instance, ifname_ext);
            mstp_cmd_show_mstp_instance_vlan_mapping(fp, p_db_inst, "                    ");
        }
    }
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
_mstp_cmd_show_mstp_debug_interface(tbl_interface_t* p_db_if, tbl_mstp_port_t *p_mstp_port, uint32 instance, FILE *fp, cfg_cmd_para_t *para)
{
    tbl_msti_port_t *p_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    bpdu_t *p_bpdu = NULL;
    uint32 has_msti = FALSE;
    char ifname_ext[IFNAME_SIZE];
    char buf[CMD_BUF_256];
    uint32 i = 0;

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    if (GLB_DEFAULT_INSTANCE == instance)
    {
        sal_fprintf(fp, "##### Interface %s, Instance %u #####\n", ifname_ext, instance);

        sal_fprintf(fp, "designatedPriority\n");
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"rootId", p_mstp_port->designated_rootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_mstp_port->designated_rootId.address));
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rootPathCost", p_mstp_port->designated_rootPathCost);
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"regionRootId", p_mstp_port->designated_regionRootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_mstp_port->designated_regionRootId.address));
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"intRootPathCost", p_mstp_port->designated_intRootPathCost);
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"designatedBridgeId", p_mstp_port->designated_designatedBridgeId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_mstp_port->designated_designatedBridgeId.address));
        sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"designatedPortId", p_mstp_port->designated_designatedPortId);
        sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"bridgePortId", p_mstp_port->designated_bridgePortId);

        sal_fprintf(fp, "portPriority\n");
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"rootId", p_mstp_port->port_rootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_mstp_port->port_rootId.address));
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rootPathCost", p_mstp_port->port_rootPathCost);
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"regionRootId", p_mstp_port->port_regionRootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_mstp_port->port_regionRootId.address));
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"intRootPathCost", p_mstp_port->port_intRootPathCost);
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"designatedBridgeId", p_mstp_port->port_designatedBridgeId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_mstp_port->port_designatedBridgeId.address));
        sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"designatedPortId", p_mstp_port->port_designatedPortId);
        sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"bridgePortId", p_mstp_port->port_bridgePortId);

        sal_fprintf(fp, "SM state\n");
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"prx_state", cdb_enum_val2str(prx_state_strs, PRX_STATE_MAX, p_mstp_port->prx_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"ppm_state", cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_mstp_port->ppm_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"bdm_state", cdb_enum_val2str(bdm_state_strs, BDM_STATE_MAX, p_mstp_port->bdm_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"ptx_state", cdb_enum_val2str(ptx_state_strs, PTX_STATE_MAX, p_mstp_port->ptx_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"pim_state", cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_mstp_port->pim_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"prt_state", cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_mstp_port->prt_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"pst_state", cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_mstp_port->pst_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"tcm_state", cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_mstp_port->tcm_state));
        sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"fea_state", cdb_enum_val2str(mstp_fea_state_strs, GLB_BR_PORT_STATE_MAX, p_mstp_port->fea_state));

        sal_fprintf(fp, "Misc\n");
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rstpVersion", p_mstp_port->rstpVersion);
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"stpVersion", p_mstp_port->stpVersion);
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rcvdInternal", p_mstp_port->rcvdInternal);
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"hopCount", p_mstp_port->hopCount);

        
        sal_fprintf(fp, "BPDU\n");
        p_bpdu = p_mstp_port->bpdu;
        if (NULL == p_bpdu)
        {
            sal_fprintf(fp, "%-20s\n", INTEND_2_STR"NULL");
        }
        else
        {
            sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"mstp_hops", p_bpdu->mstp_hops);
            sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"msti_num", p_bpdu->msti_num);
            sal_fprintf(fp, "%-20s - ", INTEND_2_STR"instance_list");
            for (i = 0; i < p_bpdu->msti_num; i++)
            {
                sal_fprintf(fp, "%-20s - %u, ", INTEND_2_STR"mstp_hops", p_bpdu->msti[i].instance_id);
            }
            sal_fprintf(fp, "\n");
        }
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_mstp_port->msti_list.obj_list, p_msti_port, listnode)
        {
            if (p_msti_port->key.instance == instance)
            {
                has_msti = TRUE;
                break;
            }
        }

        if (!has_msti)
        {
            sal_fprintf(fp, "Interface %s has no Instance %u\n", ifname_ext, instance);
        }
        else
        {
            sal_fprintf(fp, "##### Interface %s, Instance %u #####\n", ifname_ext, instance);

            sal_fprintf(fp, "designatedPriority\n");
            sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"regionRootId", p_msti_port->designated_regionRootId.priority, 
                cdb_mac_addr_val2str(buf, CMD_BUF_256, p_msti_port->designated_regionRootId.address));
            sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"intRootPathCost", p_msti_port->designated_intRootPathCost);
            sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"designatedBridgeId", p_msti_port->designated_designatedBridgeId.priority, 
                cdb_mac_addr_val2str(buf, CMD_BUF_256, p_msti_port->designated_designatedBridgeId.address));
            sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"designatedPortId", p_msti_port->designated_designatedPortId);
            sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"bridgePortId", p_msti_port->designated_bridgePortId);

            sal_fprintf(fp, "portPriority\n");
            sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"rootId", p_msti_port->port_rootId.priority, 
                cdb_mac_addr_val2str(buf, CMD_BUF_256, p_msti_port->port_rootId.address));
            sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rootPathCost", p_msti_port->port_rootPathCost);
            sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"designatedBridgeId", p_msti_port->port_designatedBridgeId.priority, 
                cdb_mac_addr_val2str(buf, CMD_BUF_256, p_msti_port->port_designatedBridgeId.address));
            sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"designatedPortId", p_msti_port->port_designatedPortId);
            sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"bridgePortId", p_msti_port->port_bridgePortId);

            sal_fprintf(fp, "SM state\n");
            sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"ppm_state", cdb_enum_val2str(ppm_state_strs, PPM_STATE_MAX, p_msti_port->ppm_state));
            sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"pim_state", cdb_enum_val2str(pim_state_strs, PIM_STATE_MAX, p_msti_port->pim_state));
            sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"prt_state", cdb_enum_val2str(prt_state_strs, PRT_STATE_MAX, p_msti_port->prt_state));
            sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"pst_state", cdb_enum_val2str(pst_state_strs, PST_STATE_MAX, p_msti_port->pst_state));
            sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"tcm_state", cdb_enum_val2str(tcm_state_strs, TCM_STATE_MAX, p_msti_port->tcm_state));
            sal_fprintf(fp, "%-20s - %s\n", INTEND_2_STR"fea_state", cdb_enum_val2str(mstp_fea_state_strs, GLB_BR_PORT_STATE_MAX, p_msti_port->fea_state));

            sal_fprintf(fp, "Misc\n");
            sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"hopCount", p_msti_port->hopCount);            
        }
    }
    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_debug_interface(tbl_interface_t* p_db_if, uint32 instance, cfg_cmd_para_t *para)
{
    tbl_mstp_port_t *p_db_port = NULL;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    p_db_port = (tbl_mstp_port_t*)p_db_if->mstp_port;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    rc = _mstp_cmd_show_mstp_debug_interface(p_db_if, p_db_port, instance, fp, para);
        
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}


int32 
_debug_compareBestPortPriority(FILE *fp, mstp_port_priority_vector_t *best_priority, tbl_mstp_port_t *p_db_port, tbl_mstp_port_t *p_db_rootport)
{
    int32 rootId_cmp = 0;
    int32 rootPathCost_cmp = 0;
    int32 regRootId_cmp = 0;
    int32 intRootPathCost_cmp = 0;
    int32 designatedBridgeId_cmp = 0;
    int32 designatedPortId_cmp = 0;
    int32 bridgePortId_cmp = 0;
    uint32 external_rpc_incr = 0;
    uint32 internal_rpc_incr = 0;
    char buf[CMD_BUF_256];

    if (p_db_port->rcvdInternal)
    {
        external_rpc_incr = 0;
        internal_rpc_incr = p_db_port->portPathCost;
    }
    else
    {
        external_rpc_incr = p_db_port->portPathCost;
        internal_rpc_incr = 0;
    }
    
    sal_fprintf(fp, "##### port %s VS rootport %s (rcvdInternal %u, RPCincr: ext %u, int %u)\n", 
        p_db_port->key.name, 
        (p_db_rootport ? (p_db_rootport->key.name) : "bridge"),
        p_db_port->rcvdInternal, external_rpc_incr, internal_rpc_incr);

    rootId_cmp = sal_memcmp(&best_priority->rootId, &p_db_port->port_rootId, sizeof(mstp_brg_id_t));
    if (rootId_cmp != 0)
    {
        sal_fprintf(fp, "1 rootId_cmp: %d\n", rootId_cmp);
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"rootId", p_db_port->port_rootId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_port->port_rootId.address));
        return rootId_cmp;
    }

    rootPathCost_cmp = best_priority->rootPathCost - (p_db_port->port_rootPathCost + external_rpc_incr);
    if (rootPathCost_cmp != 0)
    {
        sal_fprintf(fp, "2 rootPathCost_cmp: %d\n", rootPathCost_cmp);
        sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rootPathCost", (p_db_port->port_rootPathCost + external_rpc_incr));
        return rootPathCost_cmp;
    }

    if ((BPDU_VERSION_MSTP == p_db_port->forceVersion))
    {
        regRootId_cmp = sal_memcmp(&best_priority->regionRootId, &p_db_port->port_regionRootId, sizeof(mstp_brg_id_t));
        if (regRootId_cmp != 0)
        {
            sal_fprintf(fp, "3 regRootId_cmp: %d\n", regRootId_cmp);
            sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"regionRootId", p_db_port->port_regionRootId.priority, 
                cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_port->port_regionRootId.address));
            return regRootId_cmp;
        }

        intRootPathCost_cmp = best_priority->intRootPathCost - (p_db_port->port_intRootPathCost + internal_rpc_incr);
        if (intRootPathCost_cmp != 0)
        {
            sal_fprintf(fp, "4 intRootPathCost_cmp: %d\n", intRootPathCost_cmp);
            sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"intRootPathCost", (p_db_port->port_intRootPathCost + internal_rpc_incr));
            return intRootPathCost_cmp;
        }
    }

    designatedBridgeId_cmp = sal_memcmp(&best_priority->designatedBridgeId, &p_db_port->port_designatedBridgeId, sizeof(mstp_brg_id_t));
    if (designatedBridgeId_cmp != 0)
    {
        sal_fprintf(fp, "5 designatedBridgeId_cmp: %d\n", designatedBridgeId_cmp);
        sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"designatedBridgeId", p_db_port->port_designatedBridgeId.priority, 
            cdb_mac_addr_val2str(buf, CMD_BUF_256, p_db_port->port_designatedBridgeId.address));
        return designatedBridgeId_cmp;
    }

    designatedPortId_cmp = best_priority->designatedPortId - p_db_port->port_designatedPortId;
    if (designatedPortId_cmp != 0)
    {
        sal_fprintf(fp, "6 designatedPortId_cmp: %d\n", designatedPortId_cmp);
        sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"designatedPortId", p_db_port->port_designatedPortId);
        return designatedPortId_cmp;
    }

    bridgePortId_cmp = best_priority->bridgePortId - p_db_port->port_bridgePortId;
    sal_fprintf(fp, "7 bridgePortId_cmp: %d\n", bridgePortId_cmp);
    sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"bridgePortId", p_db_port->port_bridgePortId);

    return bridgePortId_cmp;
}

int32
_mstp_cmd_show_mstp_debug_tree_cist(FILE *fp, cfg_cmd_para_t *para)
{
    mstp_port_priority_vector_t bridge_priority;
    mstp_port_priority_vector_t *p_best_priority;
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_mstp_port_t *p_db_rootport = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 port_priority_cmp = 0;
    char buf[CMD_BUF_256];

    /*
    a) The root path priority vector for each Port that has a port priority vector (portPriority plus portId;
    17.19.19, 17.19.21), recorded from a received message and not aged out (infoIs == Received)
    */

    /* best_priority is inited with bridge priority vector */
    getBridgePriority(&bridge_priority, p_db_glb);
    p_best_priority = &bridge_priority;

    sal_fprintf(fp, "##### bridge_priority\n");
    sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"rootId", p_best_priority->rootId.priority, 
        cdb_mac_addr_val2str(buf, CMD_BUF_256, p_best_priority->rootId.address));
    sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"rootPathCost", p_best_priority->rootPathCost);
    sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"regionRootId", p_best_priority->regionRootId.priority, 
        cdb_mac_addr_val2str(buf, CMD_BUF_256, p_best_priority->regionRootId.address));
    sal_fprintf(fp, "%-20s - %u\n", INTEND_2_STR"intRootPathCost", p_best_priority->intRootPathCost);
    sal_fprintf(fp, "%-20s - %04x-%s\n", INTEND_2_STR"designatedBridgeId", p_best_priority->designatedBridgeId.priority, 
        cdb_mac_addr_val2str(buf, CMD_BUF_256, p_best_priority->designatedBridgeId.address));
    sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"designatedPortId", p_best_priority->designatedPortId);
    sal_fprintf(fp, "%-20s - %04x\n", INTEND_2_STR"bridgePortId", p_best_priority->bridgePortId);

    /* calculate best_priority by compare all port's priority */
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_db_port->portEnabled && (MSTP_INFO_RECEIVED == p_db_port->infoIs))
        {
            /* whose DesignatedBridgeID Bridge Address component is not equal to that component of 
               the Bridge¡¯s own bridge priority vector */
            if (0 == sal_memcmp(p_db_port->port_designatedBridgeId.address, p_db_glb->bridge_id.address, MAC_ADDR_LEN))
            {
                continue;
            }
            
            port_priority_cmp = _debug_compareBestPortPriority(fp, p_best_priority, p_db_port, p_db_rootport);
            if (port_priority_cmp < 0)
            {
                continue;
            }
            else if (port_priority_cmp == 0)
            {
                SAL_ASSERT(0);
            }
            else
            {
                /* modified by liwh for bug 37514, 2016-05-17 */
                /* Ports with Restricted Role not considered for root port selection */
                if (p_db_port->restricted_role)
                {
                    continue;    
                }
                /* liwh end */
                
                p_db_rootport = p_db_port;
                updateBestPortPriority(p_best_priority, p_db_rootport);
            }
        }
    }

    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}

int32
mstp_cmd_show_mstp_debug_tree(uint32 instance, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    if (GLB_DEFAULT_INSTANCE == instance)
    {
        inst_key.instance = instance;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
        if (NULL == p_db_inst)
        {
            CFG_CONFLICT_RET("Bridge instance %u not configured", instance);
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (GLB_DEFAULT_INSTANCE == instance)
    {
        rc = _mstp_cmd_show_mstp_debug_tree_cist(fp, para);
    }
    else
    {

    }
        
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
mstp_cmd_show_mstp_instance(uint32 instance, tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_msti_port_key_t port_key;
    tbl_msti_port_t *p_db_port = NULL;
    tbl_mstp_port_t *p_mstp_port = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    inst_key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        CFG_CONFLICT_RET("Bridge instance %u not configured", instance);
    }

    if (p_db_if)
    {
        p_mstp_port = (tbl_mstp_port_t*)p_db_if->mstp_port;
        if (NULL == p_mstp_port)
        {
            CFG_INVALID_PARAM_RET("Interface is not MSTP port");
        }        
        sal_memset(&port_key, 0, sizeof(port_key));
        sal_strcpy(port_key.name, p_db_if->key.name);
        port_key.instance = p_db_inst->key.instance;
        p_db_port = tbl_msti_port_get_msti_port(&port_key);
        if (NULL == p_db_port)
        {
            CFG_INVALID_PARAM_RET("Interface is not belong to this instance");
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    rc = _mstp_cmd_show_mstp_msti(FALSE, p_db_inst, p_db_if, fp, para);
        
    sal_fclose(fp);
    fp = NULL;
    
    return rc;
}

int32
mstp_cmd_process_mstp_show_mstp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t if_key;
    tbl_interface_t* p_db_if = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t* p_db_inst = NULL;
    uint32 is_brief = FALSE;
    uint32 is_mst = FALSE;
    uint32 is_all = FALSE;  /* all is same to up */
    uint32 instance = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        if ((0 == sal_strcmp(argv[0], "mst_brief"))
         || (0 == sal_strcmp(argv[0], "mst_detail"))
         || (0 == sal_strcmp(argv[0], "brief"))
         || (0 == sal_strcmp(argv[0], "up"))
         || (0 == sal_strcmp(argv[0], "all")))
        {
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 3, para->p_rs);
        
            if ((0 == sal_strcmp(argv[0], "mst_brief")) || (0 == sal_strcmp(argv[0], "mst_detail")))
            {
                is_mst = TRUE;
            }
            if (0 == sal_strcmp(argv[0], "brief") || (0 == sal_strcmp(argv[0], "mst_brief")))
            {
                is_brief = TRUE; 
            }
            
            if (3 == argc)
            {
                if ((0 == sal_strcmp(argv[1], "interface")))
                {
                    sal_memset(&if_key, 0, sizeof(if_key));
                    sal_strncpy(if_key.name, argv[2], IFNAME_SIZE);
                    p_db_if = tbl_interface_get_interface(&if_key);
                    if (NULL == p_db_if)
                    {
                        IFNAME_ETH2FULL(argv[2], ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                    }
                    rc = mstp_cmd_show_mstp_brief_detail_interface(p_db_if, is_mst, is_brief, para);
                }
                else if ((0 == sal_strcmp(argv[1], "instance")))
                {
                    PM_CFG_GET_UINT(instance, argv[2], para->p_rs);
                    inst_key.instance = instance;
                    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
                    if (NULL == p_db_inst)
                    {
                        CFG_INVALID_PARAM_RET("Instance %u does not exist", instance);
                    }
                    rc = mstp_cmd_show_mstp_brief_detail_instance(p_db_inst, is_mst, is_brief, para);
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid args %s", argv[1]);
                }
            }
            else
            {
                rc = mstp_cmd_show_mstp_brief_detail(is_mst, is_brief, is_all, para);
            }
        }
        else if (0 == sal_strcmp(argv[0], "mst"))
        {
            mstp_cmd_show_mstp_mst(para);
        }
        else if (0 == sal_strcmp(argv[0], "config"))
        {
            mstp_cmd_show_mstp_config(para);
        }
        else if (0 == sal_strcmp(argv[0], "instance"))
        {
            p_db_if = NULL;
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
            PM_CFG_GET_UINT(instance, argv[1], para->p_rs);
            if (3 == argc)
            {
                sal_memset(&if_key, 0, sizeof(if_key));
                sal_strncpy(if_key.name, argv[2], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&if_key);
                if (NULL == p_db_if)
                {
                    IFNAME_ETH2FULL(argv[2], ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                }
            }
            rc = mstp_cmd_show_mstp_instance(instance, p_db_if, para);
        }
        else if (0 == sal_strcmp(argv[0], "interface"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            sal_memset(&if_key, 0, sizeof(if_key));
            sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&if_key);
            if (NULL == p_db_if)
            {
                IFNAME_ETH2FULL(argv[1], ifname_ext);
                CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
            }
            rc = mstp_cmd_show_mstp_interface(p_db_if, para);
        }
        else if (0 == sal_strcmp(argv[0], "mst_debug"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            sal_memset(&if_key, 0, sizeof(if_key));
            if (sal_strlen(argv[1]))
            {
                PM_CFG_GET_UINT(instance, argv[1], para->p_rs);
            }
            sal_strncpy(if_key.name, argv[2], IFNAME_SIZE);
            p_db_if = tbl_interface_get_interface(&if_key);
            if (NULL == p_db_if)
            {
                IFNAME_ETH2FULL(argv[2], ifname_ext);
                CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
            }
            rc = mstp_cmd_show_mstp_debug_interface(p_db_if, instance, para);
        }
        else if (0 == sal_strcmp(argv[0], "mst_debug_tree"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            if (sal_strlen(argv[1]))
            {
                PM_CFG_GET_UINT(instance, argv[1], para->p_rs);
            }
            else
            {
                instance = GLB_DEFAULT_INSTANCE;
            }
            rc = mstp_cmd_show_mstp_debug_tree(instance, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid args %s", argv[0]);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}
    
#endif /* CONFIG_MSTP */
