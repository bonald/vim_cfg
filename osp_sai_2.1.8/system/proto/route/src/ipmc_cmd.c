
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_ipmc_global_define.h"
#include "gen/tbl_ipmc_global.h"
#include "gen/tbl_ipmc_intf_define.h"
#include "gen/tbl_ipmc_intf.h"
#include "gen/tbl_pim_static_rp_define.h"
#include "gen/tbl_pim_static_rp.h"
/*add by zhw for ipmc*/
#include "gen/tbl_pim_sm_define.h"
#include "gen/tbl_pim_sm.h"
#include "gen/tbl_pim_sm_bsr_define.h"
#include "gen/tbl_pim_sm_bsr.h"
#include "gen/tbl_pim_sm_rp_define.h"
#include "gen/tbl_pim_sm_rp.h"
#include "gen/tbl_pim_dm_define.h"
#include "gen/tbl_pim_dm.h"
#include "gen/tbl_igmp_intf_define.h"
#include "gen/tbl_igmp_intf.h"
#include "gen/tbl_pim_sm_rp_gp_define.h"
#include "gen/tbl_pim_sm_rp_gp.h"

/*end add*/
#include "ipmc.h"
#include "route.h"

#define PIM_SM_DEFAULT_INSTANCE   1
static int g_pim_sm_rp_gp_key = 1;

/*temp used to test code*/
uint32_t inet_parse(char *s, int n)
{
    uint32_t a = 0;
    u_int a0 = 0, a1 = 0, a2 = 0, a3 = 0;
    int i;
    char c;

    i = sscanf(s, "%u.%u.%u.%u%c", &a0, &a1, &a2, &a3, &c);
    if (i < n || i > 4 || a0 > 255 || a1 > 255 || a2 > 255 || a3 > 255)
        return 0xffffffff;

    ((uint8_t *)&a)[0] = a0;
    ((uint8_t *)&a)[1] = a1;
    ((uint8_t *)&a)[2] = a2;
    ((uint8_t *)&a)[3] = a3;

    return a;
}

static int32
_ipmc_cmd_get_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;
    tbl_ipmc_global_t      *p_db_ipmc_glb = NULL;
    int32                   field_id = 0;
    field_parse_t           field;

    sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_db_ipmc_glb = tbl_ipmc_global_get_ipmc_global();
    if (!p_db_ipmc_glb)
    {
        CFG_CONFLICT_RET("Not found tbl_ipmc_global");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    args.argv[0] = &field;
    args.argv[1] = fp;

    field_id = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
    if (field_id < 0)
    {
        CFG_DEL_SHOW_PATH(para->p_show->path, fp);
        CFG_INVALID_PARAM_RET("Invalid field");
    }

    tbl_ipmc_global_dump_one(p_db_ipmc_glb, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
ipmc_cmd_process_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    cdb_node_t            *p_node = NULL;
    tbl_ipmc_global_t     *p_db_ipmc_glb = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field = NULL;
    int32                  value = 0;
    int32                  i = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_IPMC_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_db_ipmc_glb = tbl_ipmc_global_get_ipmc_global();
        if (!p_db_ipmc_glb)
        {
            CFG_CONFLICT_RET("Not found tbl_ipmc_global");
        }

        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        switch (field_id)
        {
        case TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if(value == p_db_ipmc_glb->mcast_route_en)
            {
                break;
            }

            p_db_ipmc_glb->mcast_route_en = value;
            ipmc_api_set_ipmc_global_field(p_db_ipmc_glb, field_id, para);
            break;

        case TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if(value == p_db_ipmc_glb->register_suppress_time)
            {
                break;
            }

            p_db_ipmc_glb->register_suppress_time = value;
            ipmc_api_set_ipmc_global_field(p_db_ipmc_glb, field_id, para);
            break;

        case TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if(value == p_db_ipmc_glb->keep_alive_time)
            {
                break;
            }

            p_db_ipmc_glb->keep_alive_time = value;
            ipmc_api_set_ipmc_global_field(p_db_ipmc_glb, field_id, para);
            break;

        case TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if(value == p_db_ipmc_glb->join_prune_interval)
            {
                break;
            }

            p_db_ipmc_glb->join_prune_interval = value;
            ipmc_api_set_ipmc_global_field(p_db_ipmc_glb, field_id, para);
            break;

        case TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if(value == p_db_ipmc_glb->pim_packet_process)
            {
                break;
            }

            p_db_ipmc_glb->pim_packet_process = value;
            ipmc_api_set_ipmc_global_field(p_db_ipmc_glb, field_id, para);
            break;

        case TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

            if (!sal_strcmp(argv[i+1], "infinity"))
            {
                if (PIM_SPT_INFINITY == p_db_ipmc_glb->switchover_mode)
                {
                    break;
                }
                p_db_ipmc_glb->switchover_mode = PIM_SPT_INFINITY;
            }
            else
            {
                if (PIM_SPT_IMMEDIATE == p_db_ipmc_glb->switchover_mode)
                {
                    break;
                }
                p_db_ipmc_glb->switchover_mode = PIM_SPT_IMMEDIATE;
            }
            
            ipmc_api_set_ipmc_global_field(p_db_ipmc_glb, field_id, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _ipmc_cmd_get_global(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_ipmc_cmd_get_ipmc_intf(char *ifname, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                   *fp = NULL;
    tbl_ipmc_intf_key_t     key;
    tbl_ipmc_intf_t        *p_db_intf = NULL;
    tbl_iter_args_t         args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (NULL == ifname)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ipmc_intf_dump_one, &args);
    }
    else
    {
        sal_strncpy(key.name, ifname, IFNAME_SIZE);

        p_db_intf = tbl_ipmc_intf_get_ipmc_intf(&key);
        if (!p_db_intf)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("ipmc intf %s does not exist", ifname);
        }

        tbl_ipmc_intf_dump_one(p_db_intf, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_ipmc_cmd_create_ipmc_intf(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32               rc = PM_E_NONE;
    tbl_ipmc_intf_t     ipmc_intf;
    tbl_ipmc_intf_t    *p_db_ipmc_intf = NULL;
    char                cmd_str[128];

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(&ipmc_intf, 0, sizeof(tbl_ipmc_intf_t));
    sal_strncpy(ipmc_intf.key.name, argv[0], IFNAME_SIZE);

    p_db_ipmc_intf = tbl_ipmc_intf_get_ipmc_intf(&ipmc_intf.key);
    if (p_db_ipmc_intf)
    {
        if (!sal_strcmp(argv[1], "igmp_enable"))
        {
            if (p_db_ipmc_intf->igmp_enable)
            {
                return PM_E_NONE;
            }

            p_db_ipmc_intf->igmp_enable = 1;
            PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_ENABLE));
            sal_sprintf(cmd_str, "ip igmp");
            rt_vty_if_cmd(argv[0], cmd_str);
        }
        else
        {
            if (p_db_ipmc_intf->pim_enable)
            {
                return PM_E_NONE;
            }

            p_db_ipmc_intf->pim_enable = 1;
            PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_ENABLE));
            sal_sprintf(cmd_str, "ip pim sm");
            rt_vty_if_cmd(argv[0], cmd_str);
        }
    }
    else
    {
        if (!sal_strcmp(argv[1], "igmp_enable"))
        {
            ipmc_intf.igmp_enable = 1;
            sal_sprintf(cmd_str, "ip igmp");
            rt_vty_if_cmd(argv[0], cmd_str);
        }
        else
        {
            ipmc_intf.pim_enable = 1;
            sal_sprintf(cmd_str, "ip pim sm");
            rt_vty_if_cmd(argv[0], cmd_str);
        }

        ipmc_intf.igmp_version = IGMP_DEFAULT_VERSION;
        ipmc_intf.igmp_query_interval = IGMP_GENERAL_QUERY_INTERVAL;
        ipmc_intf.igmp_query_max_response_time = IGMP_QUERY_MAX_RESPONSE_TIME;
        ipmc_intf.pim_dr_priority = PIM_DEFAULT_DR_PRIORITY;
        ipmc_intf.pim_hello_period = PIM_DEFAULT_HELLO_PERIOD;
        ipmc_intf.pim_hello_holdtime = 3.5 * PIM_DEFAULT_HELLO_PERIOD;

        PM_E_RETURN(tbl_ipmc_intf_add_ipmc_intf(&ipmc_intf));
    }

    return rc;
}

static int32
_ipmc_cmd_delete_ipmc_intf(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32               rc = PM_E_NONE;
    tbl_ipmc_intf_t     ipmc_intf;
    tbl_ipmc_intf_t    *p_db_ipmc_intf = NULL;
    char                cmd_str[128];
    char                ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(&ipmc_intf, 0, sizeof(tbl_ipmc_intf_t));
    sal_strncpy(ipmc_intf.key.name, argv[0], IFNAME_SIZE);

    p_db_ipmc_intf = tbl_ipmc_intf_get_ipmc_intf(&ipmc_intf.key);
    if (NULL == p_db_ipmc_intf)
    {
        CFG_CONFLICT_RET("Interface %s is not configured as ipmc interface", IFNAME_ETH2FULL(ipmc_intf.key.name, ifname_ext));
    }
    
    if (!sal_strcmp(argv[1], "igmp_enable"))
    {
        if (!p_db_ipmc_intf->igmp_enable)
        {
            return PM_E_NONE;
        }

        p_db_ipmc_intf->igmp_enable = 0;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_ENABLE));
        sal_sprintf(cmd_str, "no ip igmp");
        rt_vty_if_cmd(argv[0], cmd_str);
    }
    else
    {
        if (!p_db_ipmc_intf->pim_enable)
        {
            return PM_E_NONE;
        }

        p_db_ipmc_intf->pim_enable = 0;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_ENABLE));
        sal_sprintf(cmd_str, "no ip pim sm");
        rt_vty_if_cmd(argv[0], cmd_str);
    }

    if (!p_db_ipmc_intf->igmp_enable && !p_db_ipmc_intf->pim_enable)
    {
        PM_E_RETURN(tbl_ipmc_intf_del_ipmc_intf(&p_db_ipmc_intf->key));
    }

    return rc;
}

int32
ipmc_update_ipmc_intf_delete(char *interface_name)
{
    int32               rc = PM_E_NONE;
    tbl_ipmc_intf_t     ipmc_intf;
    tbl_ipmc_intf_t    *p_db_ipmc_intf = NULL;
    char                cmd_str[128];

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(&ipmc_intf, 0, sizeof(tbl_ipmc_intf_t));
    sal_strncpy(ipmc_intf.key.name, interface_name, IFNAME_SIZE);

    p_db_ipmc_intf = tbl_ipmc_intf_get_ipmc_intf(&ipmc_intf.key);
    if (NULL == p_db_ipmc_intf)
    {
        return 0;
    }
    
    if (p_db_ipmc_intf->igmp_enable)
    {
        p_db_ipmc_intf->igmp_enable = 0;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_ENABLE));
        sal_sprintf(cmd_str, "no ip igmp");
        rt_vty_if_cmd(interface_name, cmd_str);
    }

    if (p_db_ipmc_intf->pim_enable)
    {
        p_db_ipmc_intf->pim_enable = 0;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_ENABLE));
        sal_sprintf(cmd_str, "no ip pim sm");
        rt_vty_if_cmd(interface_name, cmd_str);
    }

    if (!p_db_ipmc_intf->igmp_enable && !p_db_ipmc_intf->pim_enable)
    {
        PM_E_RETURN(tbl_ipmc_intf_del_ipmc_intf(&p_db_ipmc_intf->key));
    }

    return rc;
}


static int32
_ipmc_cmd_set_igmp_query_interval(tbl_ipmc_intf_t *p_db_ipmc_intf, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    char      cmd_str[128] = {};

    if (p_db_ipmc_intf->igmp_query_interval == value)
    {
        return rc;
    }

    if (value <= p_db_ipmc_intf->igmp_query_max_response_time) 
    {
        CFG_CONFLICT_RET("Can't set default general query interval %d sec <= query max response time %d sec.", 
            value, p_db_ipmc_intf->igmp_query_max_response_time);
    }

    p_db_ipmc_intf->igmp_query_interval = value;
    PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL));
    sal_sprintf(cmd_str, "ip igmp query-interval %d", p_db_ipmc_intf->igmp_query_interval);
    rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);

    return rc;
}

static int32
_ipmc_cmd_set_igmp_version(tbl_ipmc_intf_t *p_db_ipmc_intf, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    char      cmd_str[128] = {};

    if (!p_db_ipmc_intf->igmp_enable)
    {
        p_db_ipmc_intf->igmp_enable = 1;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_ENABLE));
        sal_sprintf(cmd_str, "ip igmp");
        rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);
    }

    if (p_db_ipmc_intf->igmp_version == value)
    {
        return rc;
    }

    p_db_ipmc_intf->igmp_version = value;
    PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_VERSION));

    sal_sprintf(cmd_str, "ip igmp version %d", p_db_ipmc_intf->igmp_version);
    rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);

    return rc;
}

static int32
_ipmc_cmd_set_igmp_query_mxrt(tbl_ipmc_intf_t *p_db_ipmc_intf, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    char      cmd_str[128] = {};

    if (p_db_ipmc_intf->igmp_query_max_response_time == value)
    {
        return rc;
    }

    if (value >= p_db_ipmc_intf->igmp_query_interval) 
    {
        CFG_CONFLICT_RET("Can't set query max response time %d sec >= general query interval %d sec.", 
            value, p_db_ipmc_intf->igmp_query_interval);
    }

    p_db_ipmc_intf->igmp_query_max_response_time = value;
    PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME));
    sal_sprintf(cmd_str, "ip igmp query-max-response-time %d", 10 * p_db_ipmc_intf->igmp_query_max_response_time);
    rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);

    return rc;
}

static int32
_ipmc_cmd_set_pim_hello_period(tbl_ipmc_intf_t *p_db_ipmc_intf, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    char      cmd_str[128] = {};

    if (p_db_ipmc_intf->pim_hello_period == value)
    {
        return rc;
    }

    p_db_ipmc_intf->pim_hello_period = value;
    PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD));

    if (!p_db_ipmc_intf->pim_hello_period || PIM_DEFAULT_HELLO_PERIOD == p_db_ipmc_intf->pim_hello_period)
    {
        p_db_ipmc_intf->pim_hello_period = PIM_DEFAULT_HELLO_PERIOD;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD));

        p_db_ipmc_intf->pim_hello_holdtime = PIM_DEFAULT_HELLO_HOLDTIME;
        PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME));
        sal_sprintf(cmd_str, "no ip pim hello");
        rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);
    }
    else
    {
        sal_sprintf(cmd_str, "ip pim hello %d", p_db_ipmc_intf->pim_hello_period);
        rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);
    }

    return rc;
}

static int32
_ipmc_cmd_set_pim_hello_holdtime(tbl_ipmc_intf_t *p_db_ipmc_intf, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    char      cmd_str[128] = {};

    if (p_db_ipmc_intf->pim_hello_holdtime == value)
    {
        return rc;
    }

    p_db_ipmc_intf->pim_hello_holdtime = value;
    PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME));

    sal_sprintf(cmd_str, "ip pim hello %d %d", p_db_ipmc_intf->pim_hello_period, p_db_ipmc_intf->pim_hello_holdtime);
    rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);
    return rc;
}

static int32
_ipmc_cmd_set_pim_dr_priority(tbl_ipmc_intf_t *p_db_ipmc_intf, int32 value, int32 field_id,
    cfg_cmd_para_t *para)
{
    int32     rc = PM_E_NONE;
    char      cmd_str[128] = {};

    if (p_db_ipmc_intf->pim_dr_priority == value)
    {
        return rc;
    }

    p_db_ipmc_intf->pim_dr_priority = value;
    PM_E_RETURN(tbl_ipmc_intf_set_ipmc_intf_field(p_db_ipmc_intf, TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY));

    sal_sprintf(cmd_str, "ip pim drpriority %d", p_db_ipmc_intf->pim_dr_priority);
    rt_vty_if_cmd(p_db_ipmc_intf->key.name, cmd_str);

    return rc;
}

int32
ipmc_cmd_process_intf(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    tbl_ipmc_intf_t        ipmc_intf;
    tbl_ipmc_intf_t       *p_db_ipmc_intf = NULL;
    tbl_interface_t        intf;
    tbl_interface_t       *p_db_if = NULL;
    tbl_route_if_t         rt_if;
    tbl_route_if_t        *p_db_rtif = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field  = NULL;
    int32                  value    = 0;
    int32                  i        = 0;
    char                   ifname_ext[IFNAME_EXT_SIZE];
    field_parse_t          field;

    CFG_DBG_FUNC();

    sal_memset(&intf, 0, sizeof(tbl_ipmc_intf_t));

    p_node = cdb_get_tbl(TBL_IPMC_INTF);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }
        sal_memset(&rt_if, 0, sizeof(rt_if));
        sal_strcpy(rt_if.key.name, intf.key.name);

        p_db_rtif = tbl_route_if_get_route_if(&rt_if.key);
        if (NULL == p_db_rtif)
        {
            CFG_CONFLICT_RET("Interface %s is not routed port", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }

        if (p_db_if->hw_type == GLB_IF_TYPE_LOOPBACK_IF)
        {
            CFG_CONFLICT_RET("Loopback interface %s can't be enabled ipmc", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }

        rc = _ipmc_cmd_create_ipmc_intf(argv, argc, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);

        rc = _ipmc_cmd_delete_ipmc_intf(argv, argc, para);
        break;

    case CDB_OPER_SET:
        sal_memset(&intf, 0, sizeof(intf));
        sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);

        p_db_if = tbl_interface_get_interface(&intf.key);
        if (!p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not found", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }

        sal_memset(&ipmc_intf, 0, sizeof(ipmc_intf));
        sal_strncpy(ipmc_intf.key.name, argv[0], IFNAME_SIZE);

        p_db_ipmc_intf = tbl_ipmc_intf_get_ipmc_intf(&ipmc_intf.key);
        if (NULL == p_db_ipmc_intf)
        {
            CFG_CONFLICT_RET("Interface %s is not configured as ipmc interface", IFNAME_ETH2FULL(intf.key.name, ifname_ext));
        }

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

        switch (field_id)
        {
        case TBL_IPMC_INTF_FLD_IGMP_VERSION:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipmc_cmd_set_igmp_version(p_db_ipmc_intf, value, field_id, para);
            break;

        case TBL_IPMC_INTF_FLD_IGMP_QUERY_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipmc_cmd_set_igmp_query_interval(p_db_ipmc_intf, value, field_id, para);
            break;

        case TBL_IPMC_INTF_FLD_IGMP_QUERY_MAX_RESPONSE_TIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipmc_cmd_set_igmp_query_mxrt(p_db_ipmc_intf, value, field_id, para);
            break;

        case TBL_IPMC_INTF_FLD_PIM_HELLO_PERIOD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if (value > p_db_ipmc_intf->pim_hello_holdtime)
            {
                CFG_CONFLICT_RET("Hello Interval can't be larger than Holdtime");
            }

            rc = _ipmc_cmd_set_pim_hello_period(p_db_ipmc_intf, value, field_id, para);
            break;

        case TBL_IPMC_INTF_FLD_PIM_HELLO_HOLDTIME:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            if (value >0 && value < p_db_ipmc_intf->pim_hello_period)
            {
                CFG_CONFLICT_RET("Hello Holdtime can't be samller than Interval");
            }

            rc = _ipmc_cmd_set_pim_hello_holdtime(p_db_ipmc_intf, value, field_id, para);
            break;

        case TBL_IPMC_INTF_FLD_PIM_DR_PRIORITY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[i+1], para->p_rs);

            rc = _ipmc_cmd_set_pim_dr_priority(p_db_ipmc_intf, value, field_id, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }

        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _ipmc_cmd_get_ipmc_intf(argv[0], para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_ipmc_cmd_check_group_range_overlap(prefix_t group_range)
{
    int32 rc = PM_E_NONE;

    //PM_E_NOT_PERMIT

    return rc;
}

static int32
_ipmc_cmd_get_static_rp_param(tbl_pim_static_rp_t *p_rp, char **argv,
            int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  rp_addr[64];
    char  group_addr[64];
    int32 i = 0;

    PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
    if (sal_strcmp(argv[i], "rp_addr"))
    {
        CFG_INVALID_PARAM_RET("Invalid param")
    }
    rc = sal_inet_pton(AF_INET, argv[++i], rp_addr);
    if (!rc)
    {
        CFG_INVALID_PARAM_RET("Invalid cli IP address");
    }
    sal_memcpy(&p_rp->rp_addr, rp_addr, sizeof(addr_ipv4_t));

    if (sal_strcmp(argv[++i], "group"))
    {
        CFG_INVALID_PARAM_RET("Invalid param");
    }

    if (NULL == argv[i+1])
    {
        sal_inet_pton(AF_INET, "224.0.0.0", group_addr);
        sal_memcpy(&(p_rp->key.group_range.u.prefix4), group_addr, sizeof(addr_ipv4_t));
        p_rp->key.group_range.prefixlen = 4;
    }
    else
    {
        rc = sal_inet_pton(AF_INET, argv[++i], group_addr);
        if (!rc)
        {
            CFG_INVALID_PARAM_RET("Invalid cli IP address");
        }
        sal_memcpy(&(p_rp->key.group_range.u.prefix4), group_addr, sizeof(addr_ipv4_t));
        p_rp->key.group_range.prefixlen = sal_atoi(argv[++i]);
    }

    return rc;
}

static int32
_ipmc_cmd_create_static_rp(tbl_pim_static_rp_t *p_rp, tbl_pim_static_rp_t *p_db_rp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  ip_addr[64];
    char  rp_addr[64];
    char  cmd_str[128];

    if (!IN_MULTICAST(sal_ntoh32(p_rp->key.group_range.u.prefix4.s_addr)))
    {
        CFG_CONFLICT_RET("Bad group address specified %s/%d", 
            cdb_addr_ipv4_val2str(ip_addr, 64, &p_rp->key.group_range.u.prefix4), p_rp->key.group_range.prefixlen);
    }

    if (IN_MULTICAST(sal_ntoh32(p_rp->rp_addr.s_addr))
        || IPV4_NET0(sal_ntoh32(p_rp->rp_addr.s_addr))
        || IPV4_NET127(sal_ntoh32(p_rp->rp_addr.s_addr))
        || IPV4_BROADCAST(sal_ntoh32(p_rp->rp_addr.s_addr)))
    {
        CFG_CONFLICT_RET("Illegal RP address");
    }

    p_db_rp = tbl_pim_static_rp_get_pim_static_rp(&(p_rp->key));
    if (NULL != p_db_rp)
    {
        return PM_E_NONE;
    }
    else
    {
        if (_ipmc_cmd_check_group_range_overlap(p_rp->key.group_range))
        {
            CFG_CONFLICT_RET("Group range specified cannot overlap");
        }

        PM_E_RETURN(tbl_pim_static_rp_add_pim_static_rp(p_rp));
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "ip pim rp %s %s/%d", 
                    cdb_addr_ipv4_val2str(rp_addr, 64, &p_rp->rp_addr),
                    cdb_addr_ipv4_val2str(ip_addr, 64, &p_rp->key.group_range.u.prefix4), p_rp->key.group_range.prefixlen);
        rt_vty_config_cmd(cmd_str);
    }

    return rc;
}

static int32
_ipmc_cmd_delete_static_rp(tbl_pim_static_rp_t *p_rp, tbl_pim_static_rp_t *p_db_rp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char  ip_addr[64];
    char  rp_addr[64];
    char  cmd_str[128];

    p_db_rp = tbl_pim_static_rp_get_pim_static_rp(&(p_rp->key));
    if (NULL == p_db_rp)
    {
        return PM_E_NONE;
    }
    else
    {
        if (_ipmc_cmd_check_group_range_overlap(p_rp->key.group_range))
        {
            CFG_CONFLICT_RET("Group range specified cannot overlap");
        }

        PM_E_RETURN(tbl_pim_static_rp_del_pim_static_rp(&p_rp->key));
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no ip pim rp %s %s/%d", 
                    cdb_addr_ipv4_val2str(rp_addr, 64, &p_rp->rp_addr),
                    cdb_addr_ipv4_val2str(ip_addr, 64, &p_rp->key.group_range.u.prefix4), p_rp->key.group_range.prefixlen);
        rt_vty_config_cmd(cmd_str);
    }

    return rc;
}

static int32
_ipmc_cmd_get_static_rp(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_pim_static_rp_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
ipmc_cmd_process_static_rp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc       = PM_E_NONE;
    cdb_node_t            *p_node   = NULL;
    tbl_pim_static_rp_t    rp;
    tbl_pim_static_rp_t   *p_db_rp  = NULL;
    field_parse_t          field;

    CFG_DBG_FUNC();

    sal_memset(&rp, 0, sizeof(tbl_pim_static_rp_t));

    p_node = cdb_get_tbl(TBL_IPMC_INTF);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        _ipmc_cmd_get_static_rp_param(&rp, argv, argc, para);
        rc = _ipmc_cmd_create_static_rp(&rp, p_db_rp, para);
        break;

    case CDB_OPER_DEL:
        _ipmc_cmd_get_static_rp_param(&rp, argv, argc, para);
        rc = _ipmc_cmd_delete_static_rp(&rp, p_db_rp, para);
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _ipmc_cmd_get_static_rp(para, p_node, &field);
        break;

    case CDB_OPER_SET:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_ipmc_cmd_get_mrt(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_mrt_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int
if_mgr_cmd_get_status(tbl_interface_t* p_db_if)
{
    if (p_db_if->errdisable_reason != ERRDIS_NO_ERRDIS)
    {
        return 1;
    }
    else if ((GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)))
    {
        return 1;
    }
    else if ((p_db_if->flags & (GLB_IFF_UP|GLB_IFF_RUNNING)) == (GLB_IFF_UP|GLB_IFF_RUNNING))
    {
        return 1;
    }
    else
    {
        return (p_db_if->flags & GLB_IFF_UP) ? 0 : 1;
    }
}

/*modify by zhw for static mc*/
int32
ipmc_cmd_process_mrt(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    tbl_mrt_t mrt;
    int32 rc = PM_E_NONE;
	char	buf[64];
	char	buf2[64];
	tbl_mrt_key_t   mrt_key;
	new_cmd_args_key_value_t key_value[10];
	int32 key_num;
	vrf_id_t        vrf_id = 0;
	addr_ipv4_t     group;
    addr_ipv4_t     source;
	uint32_t group_check;
	tbl_mrt_t * p_mrt = NULL;
	tbl_interface_t * p_if_iif = NULL;
	tbl_interface_t *p_if_oif = NULL;
	uint32 rpf_ifindex;
	uint32 oif_ifindex;
	
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(para->p_msg->u_hdr.cfg.id);

    sal_memset(&mrt, 0, sizeof(mrt));
    
    switch (para->oper)
    {
    
    /*cmd : no ip mroute*/
    case CDB_OPER_DEL:
		sal_memset(buf, 0, sizeof(buf));
		sal_memset(buf2, 0, sizeof(buf2));
		sal_memset(key_value, 0, sizeof(key_value));
		sal_memset(&mrt_key, 0, sizeof(mrt_key));
		sal_memset(&mrt, 0, sizeof(mrt));
		
		sal_strcpy(key_value[0].key_name, "source");
		sal_strcpy(key_value[1].key_name, "group");
		key_num = 2;
		vrf_id = DEFAULT_VRF_ID;
		
		cfg_parser_cmd_args_new(argv, argc, para, key_value, key_num, FALSE);
		/*source*/
		if (0 != strlen(key_value[0].key_value))
		{
			rc = sal_inet_pton(AF_INET, key_value[0].key_value, buf);
			if (!rc)
			{
				CFG_INVALID_PARAM_RET("Invalid cli source address");
			}
			sal_memcpy(&source, buf, sizeof(addr_ipv4_t));
		}
		/*group*/
		if (0 != strlen(key_value[1].key_value))
		{
			rc = sal_inet_pton(AF_INET, key_value[1].key_value, buf2);
			
			group_check = inet_parse(key_value[1].key_value, 4);
			if((ntohl(group_check) < (0xE00000FFUL)) || !rc)
			{
				CFG_INVALID_PARAM_RET("Invalid cli group address");
			}
			sal_memcpy(&group, buf2, sizeof(addr_ipv4_t));
		}

		sal_memcpy(&mrt_key.group, &group, sizeof(group));
		sal_memcpy(&mrt_key.source, &source, sizeof(source));
		mrt_key.vrf_id = vrf_id;
		p_mrt = tbl_mrt_get_mrt(&mrt_key);	
		if(!p_mrt)
		{
			CFG_INVALID_PARAM_RET("Please make sure Source address and Group address!");
		}
		if(p_mrt->static_mroute == 0)
		{
			CFG_INVALID_PARAM_RET("You can only delete static mroute!");
		}

		sal_memcpy(&mrt.key.group, &mrt_key.group, sizeof(group));
		sal_memcpy(&mrt.key.source, &mrt_key.source, sizeof(source));
		mrt.key.vrf_id = vrf_id;
		tbl_mrt_del_mrt(&mrt);
        break;
		
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _ipmc_cmd_get_mrt(para, p_node, &field);
        break;
 
    case CDB_OPER_SET:
		sal_memset(buf, 0, sizeof(buf));
		sal_memset(buf2, 0, sizeof(buf2));
		sal_memset(key_value, 0, sizeof(key_value));
	
		sal_memset(&mrt, 0, sizeof(mrt));
		sal_strcpy(key_value[0].key_name, "source");
		sal_strcpy(key_value[1].key_name, "group");
		sal_strcpy(key_value[2].key_name, "rpf");
		sal_strcpy(key_value[3].key_name, "oif");
		sal_strcpy(key_value[4].key_name, "static");
		key_num = 5;
			
		vrf_id = DEFAULT_VRF_ID;
			
		cfg_parser_cmd_args_new(argv, argc, para, key_value, key_num, FALSE);
		/*source*/
		if (0 != strlen(key_value[0].key_value))
		{
			rc = sal_inet_pton(AF_INET, key_value[0].key_value, buf);
			if (!rc)
			{
				CFG_INVALID_PARAM_RET("Invalid cli source address");
			}
			sal_memcpy(&source, buf, sizeof(addr_ipv4_t));
		}
		/*group*/
		if (0 != strlen(key_value[1].key_value))
		{
			rc = sal_inet_pton(AF_INET, key_value[1].key_value, buf2);
			
			group_check = inet_parse(key_value[1].key_value, 4);
			if((ntohl(group_check) < (0xE00000FFUL)) || !rc)
			{
				CFG_INVALID_PARAM_RET("Invalid cli group address");
			}
			sal_memcpy(&group, buf2, sizeof(addr_ipv4_t));
		}
			
		sal_memset(&mrt_key, 0, sizeof(mrt_key));
		sal_memcpy(&mrt_key.group, &group, sizeof(group));
		sal_memcpy(&mrt_key.source, &source, sizeof(source));
		mrt_key.vrf_id = vrf_id;
		p_mrt = tbl_mrt_get_mrt(&mrt_key);
			
		if(!p_mrt){
			/*mrt key*/
			sal_memcpy(&mrt.key.group, &group, sizeof(group));
			sal_memcpy(&mrt.key.source, &source, sizeof(source));
			mrt.key.vrf_id = vrf_id;
			
			/*vrf*/
			p_if_iif = tbl_interface_get_interface_by_name(key_value[2].key_value);
			if(NULL == p_if_iif)
			{
				CFG_INVALID_PARAM_RET("Add mroute error, iif interface %s not exist!\n", key_value[2].key_value);
			}
			/*if iif potr down, sai can not excute right, maybe vrf check*/
			if(!if_mgr_cmd_get_status(p_if_iif))
			{
				CFG_INVALID_PARAM_RET("Add mroute error:iif interface %s is down!\n", key_value[2].key_value);
			}
			mrt.rpf_ifindex = p_if_iif->ifindex;
				
			/*oif*/
			GLB_BMP_INIT(mrt.oif_index_bmp);
			p_if_oif = tbl_interface_get_interface_by_name(key_value[3].key_value);
			if(NULL == p_if_oif)
			{
				CFG_INVALID_PARAM_RET("Add mroute error, oif interface %s not exist!\n", key_value[3].key_value);
			}
			/*if potr down, sai can not excute right
			if(!if_mgr_cmd_get_status(p_if_oif))
			{
				CFG_INVALID_PARAM_RET("Aad mroute error:oif interface %s is down!\n", key_value[3].key_value);
			}*/
			if(mrt.rpf_ifindex == p_if_oif->ifindex)
			{
				CFG_INVALID_PARAM_RET("Error:rpf can not euqal to oif!\n");
			}
			oif_ifindex= p_if_oif->ifindex;
			GLB_BMP_SET(mrt.oif_index_bmp, oif_ifindex);
			/*static*/
			PM_CFG_GET_UINT(mrt.static_mroute, key_value[4].key_value, para->p_rs);
			/*create mroute*/
			tbl_mrt_add_mrt(&mrt);
			break;
		}
		/*mrt (s,g) exist, update rpf and oif*/
		else {
			p_if_iif = tbl_interface_get_interface_by_name(key_value[2].key_value);
			if(NULL == p_if_iif)
			{
				CFG_INVALID_PARAM_RET("Update mroute error, iif interface %s not exist!\n", key_value[2].key_value);
			}
			/*if potr down, sai can not excute right*/
			if(!if_mgr_cmd_get_status(p_if_iif))
			{
				CFG_INVALID_PARAM_RET("Update mroute error:iif interface %s is down!\n", key_value[2].key_value);
			}
			p_mrt->rpf_ifindex = p_if_iif->ifindex;
				
			p_if_oif = tbl_interface_get_interface_by_name(key_value[3].key_value);
			if(NULL == p_if_oif)
			{
				CFG_INVALID_PARAM_RET("Update mroute error, oif interface %s not exist!\n", key_value[3].key_value);
			} 
			/*if potr down, sai can not excute right
			if(!if_mgr_cmd_get_status(p_if_oif))
			{
				CFG_INVALID_PARAM_RET("Update mroute error:oif interface %s is down!\n", key_value[2].key_value);
			}*/
			if(p_mrt->rpf_ifindex == p_if_oif->ifindex)
			{
				CFG_INVALID_PARAM_RET("Error:rpf can not euqal to oif!\n");
			}
			tbl_mrt_set_mrt_field(p_mrt, TBL_MRT_FLD_RPF_IFINDEX);

			p_mrt = tbl_mrt_get_mrt(&mrt_key);
			oif_ifindex= p_if_oif->ifindex;
			GLB_BMP_SET(p_mrt->oif_index_bmp, oif_ifindex);
			tbl_mrt_set_mrt_field(p_mrt, TBL_MRT_FLD_OIF_INDEX_BMP);

			p_mrt = tbl_mrt_get_mrt(&mrt_key);
			PM_CFG_GET_UINT(p_mrt->static_mroute, key_value[4].key_value, para->p_rs);
			tbl_mrt_set_mrt_field(p_mrt, TBL_MRT_FLD_STATIC_MROUTE);	
		}
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

//add by zhw for show ip mroute
int32
rt_cmd_process_show_ip_mroute_one(tbl_mrt_t *p_mrt, tbl_iter_args_t *pargs)
{
	FILE    *fp    = (FILE*)pargs->argv[0];
    uint32  *has_config  = (uint32*)pargs->argv[1];
    if (NULL == p_mrt)
    {
        return PM_E_NONE;
    }
	
	char source[32] = {0};
	char group[32] = {0};

	sal_inet_ntop(AF_INET, (void*)&p_mrt->key.source, source, sizeof(source));
	sal_inet_ntop(AF_INET, (void*)&p_mrt->key.group, group, sizeof(group));
	sal_fprintf(fp, "  (%s, %s)", source, group);
	/*static flag*/
	if(p_mrt->static_mroute == 1)
		sal_fprintf(fp, "  static\n");
	else
		sal_fprintf(fp, "\n");
	
	uint32 ifindex = 0;
	char ifname_rpf[IFNAME_SIZE];
	char ifname_oif[IFNAME_SIZE];
	ifname_rpf[0] = CMD_ZERO_CHAR;
	tbl_interface_get_name_by_ifindex(p_mrt->rpf_ifindex, ifname_rpf, IFNAME_SIZE);
	sal_fprintf(fp, "    Incoming interface:%s\n", ifname_rpf);
	sal_fprintf(fp, "    Outgoing interface list:\n\t");
	
	GLB_BMP_ITER_BEGIN(p_mrt->oif_index_bmp, ifindex){
		ifname_oif[0] = CMD_ZERO_CHAR;
		tbl_interface_get_name_by_ifindex(ifindex, ifname_oif, IFNAME_SIZE);
		sal_fprintf(fp, "%s  ",ifname_oif);
	}
	GLB_BMP_ITER_END(p_mrt->oif_index_bmp, ifindex);
	sal_fprintf(fp, "\n");
    *has_config = TRUE;
	sal_fprintf(fp, "!\n");
    return PM_E_NONE;
}

int32
rt_cmd_process_show_ip_mroute(char **argv, int32 argc, cfg_cmd_para_t *para)
{
	FILE *fp;
	uint32          has_config = FALSE;
    tbl_iter_args_t args;
	char	buf[64];
	char	buf2[64];
	int32	rc = PM_E_NONE;
	addr_ipv4_t     group;
    addr_ipv4_t     source;
	tbl_mrt_key_t mrt_key;
	tbl_mrt_t *mrt;
	new_cmd_args_key_value_t key_value[10];
	sal_memset(key_value, 0, sizeof(key_value));

	sal_memset(&mrt, 0, sizeof(mrt));
	sal_strcpy(key_value[0].key_name, "source");
	sal_strcpy(key_value[1].key_name, "group");
	int key_num = 2;
		
	cfg_parser_cmd_args_new(argv, argc, para, key_value, key_num, FALSE);

	CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
	
	args.argv[0] = fp;
    args.argv[1] = &has_config;

	/*show all (S,G)*/
	/*argv[0]="source",argv[1]=source_addr,agve[2]="group",argv[3]=group_addr*/
    if((0 == strlen(key_value[0].key_value)) && (0 == strlen(key_value[1].key_value)))
    {
		sal_fprintf(fp, "IP Multicast Static Routing Table All:\n");
    	tbl_mrt_iterate((TBL_ITER_CB_FUNC)rt_cmd_process_show_ip_mroute_one, &args);
	}
	/*show one (S,G)*/
	else
	{	
		sal_fprintf(fp, "IP Multicast Static Routing Table:\n");
		/*source*/
		if (0 != strlen(key_value[0].key_value))
		{
			rc = sal_inet_pton(AF_INET, key_value[0].key_value, buf);
			if (!rc)
			{
				CFG_INVALID_PARAM_RET("Invalid cli source address");
			}
			sal_memcpy(&source, buf, sizeof(addr_ipv4_t));
		}
		
		/*group*/
		if (0 != strlen(key_value[1].key_value))
		{
			rc = sal_inet_pton(AF_INET, key_value[1].key_value, buf2);
			if (!rc)
			{
				CFG_INVALID_PARAM_RET("Invalid cli group address");
			}
			sal_memcpy(&group, buf2, sizeof(addr_ipv4_t));
		}

		sal_memset(&mrt_key, 0, sizeof(mrt_key));
		sal_memcpy(&mrt_key.group, &group, sizeof(group));
		sal_memcpy(&mrt_key.source, &source, sizeof(source));
		mrt_key.vrf_id = 0;
		mrt = tbl_mrt_get_mrt(&mrt_key);
		
		if(mrt)
			rt_cmd_process_show_ip_mroute_one(mrt, &args);
	}
	
	sal_fclose(fp);
    return 0;
}

static int32
_rt_cmd_del_pim_sm_rp_gp()
{
	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_pim_sm_rp_gp_t 				*p_pim_sm_rp_gp = NULL;
	tbl_pim_sm_rp_gp_master_t 		*p_master = NULL;
    p_master = tbl_pim_sm_rp_gp_get_master();

    if (NULL == p_master)
    {
        return FALSE;
    }
    CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_rp_gp_list, p_pim_sm_rp_gp, listnode, next)
    {
       if (NULL != p_pim_sm_rp_gp)
	   {
	        tbl_pim_sm_rp_gp_del_pim_sm_rp_gp(&(p_pim_sm_rp_gp->key));
	   }
   }
   g_pim_sm_rp_gp_key = 1;
   return TRUE;
}

static int32
_rt_cmd_del_pim_dm_one(tbl_pim_dm_t *p_pim_dm, tbl_iter_args_t *pargs)
{
   // _rt_cmd_clear_pim_dm(p_pim_dm);
    return tbl_pim_dm_del_pim_dm(&p_pim_dm->key);
}


static int32
_rt_cmd_del_pim_dm_process(void)
{
    tbl_iter_args_t args;

    tbl_pim_dm_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_pim_dm_one, &args);

    return PM_E_NONE;
}

static int32
_rt_cmd_del_pim_sm_one(tbl_pim_sm_t *p_pim_sm, tbl_iter_args_t *pargs)
{
    return tbl_pim_sm_del_pim_sm(&p_pim_sm->key);
}

static int32
_rt_cmd_del_pim_sm_bsr_one(tbl_pim_sm_bsr_t *p_pim_sm_bsr, tbl_iter_args_t *pargs)
{
    return tbl_pim_sm_bsr_del_pim_sm_bsr(&p_pim_sm_bsr->key);
}

static int32
_rt_cmd_del_pim_sm_rp_one(tbl_pim_sm_rp_t *p_pim_sm_rp, tbl_iter_args_t *pargs)
{
    return tbl_pim_sm_rp_del_pim_sm_rp(&p_pim_sm_rp->key);
}

static int32
_rt_cmd_del_pim_sm_process(void)
{
    tbl_iter_args_t args;

    /*  1. delete the pim_sm bsr 
        2. delete the pim_sm rp 
        3. delete the pim_sm 
    */

    tbl_pim_sm_bsr_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_pim_sm_bsr_one, &args);
    tbl_pim_sm_rp_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_pim_sm_rp_one, &args);
    tbl_pim_sm_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_pim_sm_one, &args);
	_rt_cmd_del_pim_sm_rp_gp();

    return PM_E_NONE;
}

int32
rt_cmd_process_pim_sm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
	tbl_pim_sm_t					*p_db_pim_sm;
    tbl_pim_sm_t                    pim_sm;
    tbl_pim_sm_key_t                pim_sm_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;

    cdb_field_t 	*p_field            = NULL;
    tbl_pim_sm_t  	*p_pim_sm				= NULL;
    int32       	field_id            = 0;
	char        	buf[64];
	int32       	retv 				= 0;

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    CFG_DBG_FUNC();

    sal_memset(&field, 0, sizeof(field));
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&pim_sm, 0, sizeof(pim_sm));
    sal_memset(&pim_sm_key, 0, sizeof(pim_sm_key));
    p_node = cdb_get_tbl(TBL_PIM_SM);

    pim_sm_key.id = PIM_SM_DEFAULT_INSTANCE;
    p_pim_sm = tbl_pim_sm_get_pim_sm(&pim_sm_key);

	switch (para->oper)
	{
	case CDB_OPER_ADD:
		if (1 == argc && 0 == sal_strcmp(argv[0], "create_pim_sm"))
		{
			if (NULL == p_pim_sm)
			{
				pim_sm.key.id = PIM_SM_DEFAULT_INSTANCE;
				rc = tbl_pim_sm_add_pim_sm(&pim_sm);
			}
		}
		break;	
		
	case CDB_OPER_DEL:
        if (1 == argc && 0 == sal_strcmp(argv[0], "remove_pim_sm"))
        {
            _rt_cmd_del_pim_sm_process();
        }
        break;
		
   	case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (NULL == p_pim_sm)
        {
            pim_sm.key.id = PIM_SM_DEFAULT_INSTANCE;
            rc = tbl_pim_sm_add_pim_sm(&pim_sm);
        }
        p_db_pim_sm = tbl_pim_sm_get_pim_sm(&pim_sm_key);
        
        switch (field_id)
        {/*
         case TBL_PIM_SM_FLD_GROUP_PREFIX:
				if (3 == argc && 0 == sal_strcmp(argv[0], "group_prefix"))
	        	{
	            	sal_strcpy(p_db_pim_sm->group_prefix.ip, argv[1]);
	            	PM_CFG_GET_UINT(p_db_pim_sm->group_prefix.masklen, argv[2], para->p_rs);
	                tbl_pim_sm_set_pim_sm_field(p_db_pim_sm, TBL_PIM_SM_FLD_GROUP_PREFIX);
				}
				break;*/
	     case TBL_PIM_SM_FLD_RP_ADDR:
			  if (4 == argc && 0 == sal_strcmp(argv[0], "rp_addr"))
			  {
				  sal_strcpy(p_db_pim_sm->rp_addr.ip, argv[1]);
				  sal_strcpy(p_db_pim_sm->rp_addr.group, argv[2]); 			  
				  PM_CFG_GET_UINT(p_db_pim_sm->rp_addr.masklen, argv[3], para->p_rs);
				  tbl_pim_sm_set_pim_sm_field(p_db_pim_sm, TBL_PIM_SM_FLD_RP_ADDR);
			  }
			  break;
	      case TBL_PIM_SM_FLD_HELLO_TIME:
			 if (2 == argc && 0 == sal_strcmp(argv[0], "hello_time"))
			 {
				 
				 PM_CFG_GET_UINT(p_db_pim_sm->hello_time, argv[1], para->p_rs);
				 tbl_pim_sm_set_pim_sm_field(p_db_pim_sm, TBL_PIM_SM_FLD_HELLO_TIME);
			 }
			 break;
	      case TBL_PIM_SM_FLD_JP_TIMER:
			 if (2 == argc && 0 == sal_strcmp(argv[0], "jp_timer"))
			 {
				 
				 PM_CFG_GET_UINT(p_db_pim_sm->jp_timer, argv[1], para->p_rs);
				 tbl_pim_sm_set_pim_sm_field(p_db_pim_sm, TBL_PIM_SM_FLD_JP_TIMER);
			 }
			 break;			 
	      case TBL_PIM_SM_FLD_SPT_INFINITY:
				if (2 == argc && 0 == sal_strcmp(argv[0], "spt_infinity"))
				{
					
					PM_CFG_GET_UINT(p_db_pim_sm->spt_infinity, argv[1], para->p_rs);
					tbl_pim_sm_set_pim_sm_field(p_db_pim_sm, TBL_PIM_SM_FLD_SPT_INFINITY);
				}
			    break;

		  default:
		 	CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            break;
        }

        break;
	default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
	}
	
	return rc;
}

int32
rt_cmd_process_pim_sm_bsr(char **argv, int32 argc, cfg_cmd_para_t *para)
{
	cdb_node_t *p_node              = NULL;
	tbl_pim_sm_bsr_t            pim_sm_bsr;
	tbl_pim_sm_bsr_key_t		pim_sm_bsr_key;
	tbl_pim_sm_bsr_t			*p_db_pim_sm_bsr = NULL;
	tbl_route_if_t 				*p_db_route_if = NULL;
    int32       rc		= PM_E_NONE;
	tbl_interface_t     		*p_db_interface = NULL;
	tbl_route_if_key_t rt_if_key;
	ds_connected_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *rt_listnode = NULL;
    uint32              has_addr = 0;
	
	CFG_DBG_FUNC();
	sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memset(&pim_sm_bsr_key, 0, sizeof(pim_sm_bsr_key));
    sal_memset(&pim_sm_bsr, 0, sizeof(pim_sm_bsr));
    p_node = cdb_get_tbl(TBL_PIM_SM_BSR);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_pim_sm_bsr_t 				*p_pim_sm_bsr = NULL;
	tbl_pim_sm_bsr_master_t 		*p_master = NULL;
    p_master = tbl_pim_sm_bsr_get_master();
	if (NULL == p_master)
    {
        return rc;
    }
	
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;
		
	case CDB_OPER_DEL:
		if (0 == argc)
        {
        	CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_bsr_list, p_pim_sm_bsr, listnode, next)
            {
            	if (NULL != p_pim_sm_bsr)
	            {
	                tbl_pim_sm_bsr_del_pim_sm_bsr(&(p_pim_sm_bsr->key));
	            }
            }
            
        }
		break;

	case CDB_OPER_SET:	
		/*add by zhw for check port exist*/	
		p_db_interface = tbl_interface_get_interface_by_name(argv[0]);
		if(p_db_interface == NULL)
		{
			CFG_INVALID_PARAM_RET("Interface %s does not exist!", argv[0]);
		}
		if(p_db_interface->mode != GLB_IF_MODE_L3)
		{
			CFG_INVALID_PARAM_RET("Interface %s is not L3 interface!", argv[0]);
		}
    	
    	sal_strncpy(rt_if_key.name, argv[0], IFNAME_SIZE);
   	 	p_db_route_if = tbl_route_if_get_route_if(&rt_if_key);
		if(p_db_route_if == NULL)
		{
			CFG_INVALID_PARAM_RET("zhw test :Interface %s do not have ip!", argv[0]);
		}
		CTCLIB_SLIST_LOOP(p_db_route_if->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
    	{
        	if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        	{
        		has_addr = 1;
        	}
    	}
		if (!has_addr)
		{
			CFG_INVALID_PARAM_RET("Interface %s do not have ip!", argv[0]);
		}
		
		/*end*/
		CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_bsr_list, p_pim_sm_bsr, listnode, next)
        {
            	if (NULL != p_pim_sm_bsr)
	            {
	                tbl_pim_sm_bsr_del_pim_sm_bsr(&(p_pim_sm_bsr->key));
	            }
        }
		if (5 == argc)
        {
        	sal_strncpy(pim_sm_bsr_key.name, argv[0], IFNAME_SIZE);
    		p_db_pim_sm_bsr = tbl_pim_sm_bsr_get_pim_sm_bsr(&pim_sm_bsr_key);
			if(NULL == p_db_pim_sm_bsr)
			{
				 sal_memcpy(&pim_sm_bsr.key, &pim_sm_bsr_key, sizeof(pim_sm_bsr_key));
                 tbl_pim_sm_bsr_add_pim_sm_bsr(&pim_sm_bsr);
				 p_db_pim_sm_bsr = tbl_pim_sm_bsr_get_pim_sm_bsr(&pim_sm_bsr_key);
			}
            if (NULL != p_db_pim_sm_bsr)
            {
				PM_CFG_GET_UINT(p_db_pim_sm_bsr->priority, argv[2], para->p_rs);
				tbl_pim_sm_bsr_set_pim_sm_bsr_field(p_db_pim_sm_bsr, TBL_PIM_SM_BSR_FLD_PRIORITY);
				PM_CFG_GET_UINT(p_db_pim_sm_bsr->interval, argv[4], para->p_rs);
				tbl_pim_sm_bsr_set_pim_sm_bsr_field(p_db_pim_sm_bsr, TBL_PIM_SM_BSR_FLD_INTERVAL);
            }
        }
		break;
		
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_pim_sm_rp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
	cdb_node_t *p_node              = NULL;
	tbl_pim_sm_rp_t            	pim_sm_rp;
	tbl_pim_sm_rp_key_t			pim_sm_rp_key;
	tbl_pim_sm_rp_t				*p_db_pim_sm_rp = NULL;
	
	tbl_route_if_t 				*p_db_route_if = NULL;
	tbl_interface_t     		*p_db_interface = NULL;
	tbl_route_if_key_t rt_if_key;
	ds_connected_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *rt_listnode = NULL;
    uint32              has_addr = 0;
	
	tbl_pim_sm_t  *p_pim_sm				= NULL;

    int32       rc		= PM_E_NONE;
	char        buf[64];

	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&pim_sm_rp_key, 0, sizeof(pim_sm_rp_key));
    sal_memset(&pim_sm_rp, 0, sizeof(pim_sm_rp));
    p_node = cdb_get_tbl(TBL_PIM_SM_RP);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_pim_sm_rp_t 				*p_pim_sm_rp = NULL;
	tbl_pim_sm_rp_master_t 		*p_master = NULL;
    p_master = tbl_pim_sm_rp_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 == argc)
        {
   			sal_strncpy(pim_sm_rp_key.name, argv[0], IFNAME_SIZE);
            p_db_pim_sm_rp = tbl_pim_sm_rp_get_pim_sm_rp(&pim_sm_rp_key);
            if (NULL == p_db_pim_sm_rp)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_rp_list, p_pim_sm_rp, listnode, next)
                {
                    if (0 == sal_memcmp(p_pim_sm_rp->key.name, pim_sm_rp_key.name,sizeof(p_pim_sm_rp->key.name)))
                    {
                        return rc;
                    }                    
                }
                sal_memcpy(&pim_sm_rp.key, &pim_sm_rp_key, sizeof(pim_sm_rp_key));
                tbl_pim_sm_rp_add_pim_sm_rp(&pim_sm_rp);
            }
        }
        break;
		
	case CDB_OPER_DEL:
		if (0 == argc)
        {
        	CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_rp_list, p_pim_sm_rp, listnode, next)
            {
            	if (NULL != p_pim_sm_rp)
	            {
	                tbl_pim_sm_rp_del_pim_sm_rp(&(p_pim_sm_rp->key));
	            }
            }
            
        }
		break;

	case CDB_OPER_SET:
		/*add by zhw for check port exist*/	
		p_db_interface = tbl_interface_get_interface_by_name(argv[0]);
		if(p_db_interface == NULL)
		{
			CFG_INVALID_PARAM_RET("Interface %s does not exist!", argv[0]);
		}
		if(p_db_interface->mode != GLB_IF_MODE_L3)
		{
			CFG_INVALID_PARAM_RET("Interface %s is not L3 interface!", argv[0]);
		}
    	
    	sal_strncpy(rt_if_key.name, argv[0], IFNAME_SIZE);
   	 	p_db_route_if = tbl_route_if_get_route_if(&rt_if_key);
		if(p_db_route_if == NULL)
		{
			CFG_INVALID_PARAM_RET("zhw test :Interface %s do not have ip!", argv[0]);
		}
		CTCLIB_SLIST_LOOP(p_db_route_if->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
    	{
        	if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        	{
        		has_addr = 1;
        	}
    	}
		if (!has_addr)
		{
			CFG_INVALID_PARAM_RET("Interface %s do not have ip!", argv[0]);
		}
		/*end*/
		
		CTCLIB_SLIST_LOOP_DEL(p_master->pim_sm_rp_list, p_pim_sm_rp, listnode, next)
        {
          	if (NULL != p_pim_sm_rp)
	        {
	            tbl_pim_sm_rp_del_pim_sm_rp(&(p_pim_sm_rp->key));
	        }
        }
		if (5 == argc)
        {
   			sal_strncpy(pim_sm_rp_key.name, argv[0], IFNAME_SIZE);
            p_db_pim_sm_rp = tbl_pim_sm_rp_get_pim_sm_rp(&pim_sm_rp_key);
			if(NULL == p_db_pim_sm_rp)
			{
				sal_memcpy(&pim_sm_rp.key, &pim_sm_rp_key, sizeof(pim_sm_rp_key));
                tbl_pim_sm_rp_add_pim_sm_rp(&pim_sm_rp);
				p_db_pim_sm_rp = tbl_pim_sm_rp_get_pim_sm_rp(&pim_sm_rp_key);
			}
            if (NULL != p_db_pim_sm_rp)
            {
				PM_CFG_GET_UINT(p_db_pim_sm_rp->priority, argv[2], para->p_rs);
				tbl_pim_sm_rp_set_pim_sm_rp_field(p_db_pim_sm_rp, TBL_PIM_SM_RP_FLD_PRIORITY);
				PM_CFG_GET_UINT(p_db_pim_sm_rp->interval, argv[4], para->p_rs);
				tbl_pim_sm_rp_set_pim_sm_rp_field(p_db_pim_sm_rp, TBL_PIM_SM_RP_FLD_INTERVAL);
            }
        }
		break;
		
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_pim_sm_rp_gp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
	tbl_pim_sm_rp_gp_t            	pim_sm_rp_gp;
	tbl_pim_sm_rp_gp_key_t			pim_sm_rp_gp_key;
	tbl_pim_sm_rp_gp_t				*p_db_pim_sm_rp_gp = NULL;


    int32       rc		= PM_E_NONE;
	char        buf[64];
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&pim_sm_rp_gp_key, 0, sizeof(pim_sm_rp_gp_key));
    sal_memset(&pim_sm_rp_gp, 0, sizeof(pim_sm_rp_gp));

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (3 == argc && 0 == sal_strcmp(argv[0], "group_prefix"))
	    {
			pim_sm_rp_gp.key.id = g_pim_sm_rp_gp_key;
    		tbl_pim_sm_rp_gp_add_pim_sm_rp_gp(&pim_sm_rp_gp);
			pim_sm_rp_gp_key.id = g_pim_sm_rp_gp_key;
			g_pim_sm_rp_gp_key++;
			
			p_db_pim_sm_rp_gp = tbl_pim_sm_rp_gp_get_pim_sm_rp_gp(&pim_sm_rp_gp_key);
	    	sal_strcpy(p_db_pim_sm_rp_gp->group_prefix.ip, argv[1]);
	        PM_CFG_GET_UINT(p_db_pim_sm_rp_gp->group_prefix.masklen, argv[2], para->p_rs);
	        tbl_pim_sm_rp_gp_set_pim_sm_rp_gp_field(p_db_pim_sm_rp_gp, TBL_PIM_SM_RP_GP_FLD_GROUP_PREFIX);
		}
        break;
		
	case CDB_OPER_DEL:
		if (0 == argc)
        {
        	_rt_cmd_del_pim_sm_rp_gp();
        }
		break;	
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_pim_dm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
	tbl_pim_dm_t					*p_db_pim_dm;
    tbl_pim_dm_t                    pim_dm;
    tbl_pim_dm_key_t                pim_dm_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;

    cdb_field_t 	*p_field            = NULL;
    tbl_pim_dm_t  	*p_pim_dm				= NULL;
    int32       	field_id            = 0;
	char        	buf[64];
	int32       	retv 				= 0;

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    CFG_DBG_FUNC();

    sal_memset(&field, 0, sizeof(field));
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&pim_dm, 0, sizeof(pim_dm));
    sal_memset(&pim_dm_key, 0, sizeof(pim_dm_key));
    p_node = cdb_get_tbl(TBL_PIM_SM);

    pim_dm_key.id = PIM_SM_DEFAULT_INSTANCE;
    p_pim_dm = tbl_pim_dm_get_pim_dm(&pim_dm_key);

	switch (para->oper)
	{
	case CDB_OPER_ADD:
		if (1 == argc && 0 == sal_strcmp(argv[0], "create_pim_dm"))
		{
			if (NULL == p_pim_dm)
			{
				pim_dm.key.id = PIM_SM_DEFAULT_INSTANCE;
				rc = tbl_pim_dm_add_pim_dm(&pim_dm);
			}
		}
		break;	
		
	case CDB_OPER_DEL:
        if (1 == argc && 0 == sal_strcmp(argv[0], "remove_pim_dm"))
        {
            _rt_cmd_del_pim_dm_process();
        }
        break;
		
   	case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (NULL == p_pim_dm)
        {
            pim_dm.key.id = PIM_SM_DEFAULT_INSTANCE;
            rc = tbl_pim_dm_add_pim_dm(&pim_dm);
        }
        p_db_pim_dm = tbl_pim_dm_get_pim_dm(&pim_dm_key);
        

        break;
	default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
	}
	
	return rc;
}

int32
rt_cmd_process_igmp_intf(char **argv, int32 argc, cfg_cmd_para_t *para)
{

	cdb_node_t *p_node              = NULL;
	tbl_igmp_intf_t            igmp_intf;	/*local data for tran to db*/
	tbl_igmp_intf_key_t		igmp_intf_key;
	tbl_igmp_intf_t			*p_db_igmp_intf = NULL;
	tbl_igmp_intf_t			*p_igmp_intf = NULL;
    int32       	field_id            = 0;
    cdb_field_t 	*p_field            = NULL;
	tbl_interface_t *p_db_if        = NULL;
	
    int32       rc		= PM_E_NONE;	
    sal_memset(&igmp_intf, 0, sizeof(igmp_intf));

	/*get data by input key*/
	sal_memset(&igmp_intf_key, 0, sizeof(igmp_intf_key));
	sal_strcpy(igmp_intf_key.name,argv[0]);
	p_igmp_intf=tbl_igmp_intf_get_igmp_intf(&igmp_intf_key);
    p_node = cdb_get_tbl(TBL_IGMP_INTF);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
       /*no op*/
        break;
		
	case CDB_OPER_DEL:
		if(p_igmp_intf)
	    	tbl_igmp_intf_del_igmp_intf(&(p_igmp_intf->key));
	    break;

	case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

		 if (NULL == p_igmp_intf)
        {
			sal_strcpy(igmp_intf.key.name,argv[0]);
            rc = tbl_igmp_intf_add_igmp_intf(&igmp_intf);
        }
		 
        p_db_igmp_intf = tbl_igmp_intf_get_igmp_intf(&igmp_intf_key);
		
        switch (field_id)
        {
	     case TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT:
			  if (3 == argc && 0 == sal_strcmp(argv[1], "querier_timeout"))
			  {			 
				  PM_CFG_GET_UINT(p_db_igmp_intf->querier_timeout, argv[2], para->p_rs);
				  tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT);
			  }
			  break;
	      case TBL_IGMP_INTF_FLD_QUERY_INTERVAL:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "query_interval"))
			 {
				 
				 PM_CFG_GET_UINT(p_db_igmp_intf->query_interval, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_QUERY_INTERVAL);
			 }
			 break;
		  case TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE:
				if (3 == argc && 0 == sal_strcmp(argv[1], "robustness_variable"))
				{
					PM_CFG_GET_UINT(p_db_igmp_intf->robustness_variable, argv[2], para->p_rs);
					tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE); 
				}
				break;
		  case TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "layer3_protocol_index"))
			 {
				 PM_CFG_GET_UINT(p_db_igmp_intf->layer3_protocol_index, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX);
			 }
			 break;
		  case TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "layer3_protocol"))
			 {	 
				 PM_CFG_GET_UINT(p_db_igmp_intf->layer3_protocol, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL);
			 }
			 break;
		  case TBL_IGMP_INTF_FLD_ENTRY_VALID:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "entry_valid"))
			 {
				 PM_CFG_GET_UINT(p_db_igmp_intf->entry_valid, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_ENTRY_VALID);
			 }
			 break;
		  case TBL_IGMP_INTF_FLD_ACTION_INDEX:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "action_index"))
			 {
				 PM_CFG_GET_UINT(p_db_igmp_intf->action_index, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_ACTION_INDEX);
			 }
			 break;
		 case TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "last_member_query_count"))
			 {
				 PM_CFG_GET_UINT(p_db_igmp_intf->last_member_query_count, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT);
			 }
			 break;
		case TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "last_member_query_interval"))
			 {
				 PM_CFG_GET_UINT(p_db_igmp_intf->last_member_query_interval, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL);
			 }
			 break;
		case TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER:
			 if (3 == argc && 0 == sal_strcmp(argv[1], "global_limit_number"))
			 {
				 PM_CFG_GET_UINT(p_db_igmp_intf->global_limit_number, argv[2], para->p_rs);
				 tbl_igmp_intf_set_igmp_intf_field(p_db_igmp_intf, TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER);
			 }
			 break;
		  default:
		 	CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            break;
        }
			break;
    	default:
       		 CFG_INVALID_PARAM_RET("Invalid action");
       		 break;
    }

    return rc;
}


