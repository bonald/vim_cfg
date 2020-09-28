#include "proto.h"
#include "switch.h"
#include "glb_ns_port_forwarding.h"

#include "ns_route.h"
#include "gen/tbl_ns_route_define.h"
#include "gen/tbl_ns_route.h"
#include "gen/tbl_ns_route_ip_define.h"
#include "gen/tbl_ns_route_ip.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"

/**
 * call "ip route" to add or delete route
 * @param ip: remote ip address
 * @param is_add: add route if it is true, else delete route
 * @return: return 0 if operation successful
 */
static int32
_ns_route_ip_oper_route(addr_t *ip, bool is_add)
{
    char ip_buf[GLB_IPV6_STR_LEN] = "";
    char oper_str[10] = "";
    char cmd[1024];

    if (AF_INET == ip->family)
    {
        sal_inet_ntop(AF_INET, (void*)&ip->u.prefix4, ip_buf, sizeof(ip_buf));
    }
    else
    {
        sal_inet_ntop(AF_INET6, (void*)&ip->u.prefix6, ip_buf, sizeof(ip_buf));    
    }

    if (is_add)
    {
        sal_strcpy(oper_str, "add"); 
    }
    else
    {
        sal_strcpy(oper_str, "del"); 
    }

    if (AF_INET == ip->family)
    {
        sprintf(cmd, "ip route %s %s via %s",
            oper_str,
            ip_buf,
            GLB_NS_ROUTE_MGMT_IP_ADDR);
    }
    else
    {
        sprintf(cmd, "ip route %s %s via %s dev %s src %s",
            oper_str,
            ip_buf,
            GLB_NS_PORT_FORWARDING_VETH_MGMT2_IPV6_ADDR,
            /* modified by liwh for bug 49599, 2018-10-22 
                 should set src ipv6 address, otherwise will use inband ipv6 address */
            "vedefault2",
            GLB_NS_PORT_FORWARDING_VETH_DEFAULT2_IPV6_ADDR);
            /* liwh end */
    }

    return sal_system(cmd);
}


static int32
_ns_route_ip_update_counter(cfg_cmd_para_t *para, uint32 is_inband, addr_t *ip, bool is_incr, uint32 ns_rt_flag)
{
    int32 rc = PM_E_NONE;

    tbl_ns_route_ip_key_t key;
    tbl_ns_route_ip_t *p_ns_route_ip;
    tbl_ns_route_ip_t ns_route_ip;

    sal_memset(&key, 0, sizeof(tbl_ns_route_ip_key_t));
    sal_memcpy(&key.ip, ip, sizeof(addr_t));
    p_ns_route_ip = tbl_ns_route_ip_get_ns_route_ip(&key);

    if (NULL == p_ns_route_ip)
    {
        if (is_incr)  /* create a new entry */
        {
            sal_memcpy(&ns_route_ip.key, &key, sizeof(tbl_ns_route_ip_key_t));
            ns_route_ip.cnt = 1;
            ns_route_ip.is_inband = is_inband;
            
            /* add route if it is outband */
            if (!is_inband && 0 != _ns_route_ip_oper_route(&ns_route_ip.key.ip, TRUE))
            {
                CFG_INVALID_PARAM_RET("Can not add route");
            }

            /* add entry to db */
            PM_E_RETURN(tbl_ns_route_ip_add_ns_route_ip(&ns_route_ip));
        }
        else
        {
            CFG_INVALID_PARAM_RET("Can not delete route");
        }
    }
    else
    {
        /* check conflict */
        if (is_inband != p_ns_route_ip->is_inband) // && NS_ROUTE_FLAG_SNMP_SERVER != ns_rt_flag)
        {
            CFG_INVALID_PARAM_RET("This address already assign to %s", p_ns_route_ip->is_inband ? "inband" : "outband");
        }

        if (is_incr)
        {
            p_ns_route_ip->cnt++;
            PM_E_RETURN(tbl_ns_route_ip_set_ns_route_ip_field(p_ns_route_ip, TBL_NS_ROUTE_IP_FLD_CNT));
        }
        else
        {
            p_ns_route_ip->cnt--;
            if (p_ns_route_ip->cnt <= 0) /* delete this entry */
            {
                if (!is_inband && 0 != _ns_route_ip_oper_route(&p_ns_route_ip->key.ip, FALSE)) /* delete route if it is outband */
                {
                    CFG_INVALID_PARAM_RET("Can not delete route");
                }
                PM_E_RETURN(tbl_ns_route_ip_del_ns_route_ip(&p_ns_route_ip->key));
            }
            else
            {
                PM_E_RETURN(tbl_ns_route_ip_set_ns_route_ip_field(p_ns_route_ip, TBL_NS_ROUTE_IP_FLD_CNT));
            }
        }
    }

    return rc;
}


static int32
_ns_route_ip_dump_all(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
 
    tbl_ns_route_ip_iterate((TBL_ITER_CB_FUNC)tbl_ns_route_ip_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
ns_route_ip_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    
    CFG_DBG_FUNC();

    switch (para->oper)
    {
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            PM_E_RETURN(_ns_route_ip_dump_all(para, &field));
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}


static int32
_ns_route_add_route_convert_str_into_flag_enum(cfg_cmd_para_t *para, const char *str, ns_route_flag_t *flag)
{
    int32 rc = PM_E_NONE;

    *flag = sal_atoi(str);

    if (NS_ROUTE_FLAG_INVALID == *flag || *flag >= NS_ROUTE_FLAG_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid flag %s", str);
    }

    return rc;
}


static int32
_ns_route_convert_args_into_key(cfg_cmd_para_t *para, const char *str, tbl_ns_route_key_t *p_key)
{
    int32 rc = PM_E_NONE;
    char ip_buf[GLB_IPV6_STR_LEN] = "";
    int  proto;  /* 0-tcp, 1-udp */

    sal_memset(p_key, 0, sizeof(tbl_ns_route_key_t));

    if (3 != sal_sscanf(str, "%d#%hu#%s", &proto, &p_key->port, ip_buf))
    {
        CFG_INVALID_PARAM_RET("Invalid parameter %s", str);
    }

    /* verify params */
    p_key->proto = proto;
    if (p_key->proto >= NS_ROUTE_MAX)
    {
        CFG_INVALID_PARAM_RET("Invalid protocol: %d", (int)p_key->proto);
    }

    if (PM_E_NONE != cdb_ns_route_key_str2val(ip_buf, p_key))
    {
        CFG_INVALID_PARAM_RET("Invalid ip address: %s", ip_buf);
    }

    return rc;
}

/* modified by liwh for bug 49264, 2018-10-08 */
uint32
ipv6_api_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) { 
        return 0;
    }

    type = p_chassis->cur_stm_mode;    
    if (type < GLB_STM_MODE_MAX) {
        if (GLB_STM_MODE_IPUCV6 == type)
        {
            p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
            if(NULL != p_sys_spec) {
                return p_sys_spec->host_v6_routes;
            }
        }
    }
            
    return 0;
}
/* liwh end */

/* modified by liwh for bug 53215, 2019-08-15 */
uint32
ipv6_api_count_get_max_for_gg_gb(uint32   is_inband, tbl_ns_route_key_t *p_key)
{
    /*modified by chenc for bug 53945, 2019-10-17*/
//#if defined (GOLDENGATE) || defined (GREATBELT)
    /*chenc end*/
    /* in band server is not supported when there is no ipv6 resource */
    if (is_inband)
    {
        return FALSE;    
    }

    return TRUE;
    /*modified by chenc for bug 53945, 2019-10-17*/
//#endif

    //return FALSE;
    /*chenc end*/
}
/* liwh end */

static int32
_ns_route_add_route(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_ns_route_key_t key;
    tbl_ns_route_t ns_route;
    tbl_ns_route_t *p_ns_route;
    ns_route_flag_t flag;
    prefix_t             address;

    sal_memset(&ns_route, 0, sizeof(tbl_ns_route_t));
    sal_memset(&address, 0, sizeof(prefix_t));

    /* check params */
    /* TODO: ifname is required */
    if (3 != argc && 4 != argc) /* must be [$protocol#$remote_port#$remote_ip, $flag, inband | outband, [ifname] ] */
    {
        CFG_INVALID_PARAM_RET("Invalid parameter");
    }

    /* get ifname */
    if (4 == argc)
    {
        sal_strcpy(ns_route.ifname, argv[3]);
    }
    else
    {
        sal_strcpy(ns_route.ifname, "default");
    }

    /* check if is inband */
    if (0 == sal_strcmp(argv[2], "inband"))
    {
        ns_route.is_inband = TRUE;
    }
    else
    {
        ns_route.is_inband = FALSE;
    }

    /* convert params into key */
    PM_E_RETURN(_ns_route_convert_args_into_key(para, argv[0], &key));

    /* modified by liwh for bug 49264, 2018-10-08 */
    if (AF_INET6 == key.ip.family)
    {
        if (0 == ipv6_api_count_get_max())
        {
            /* modified by liwh for bug 53215, 2019-08-15 */
            if (0 == ipv6_api_count_get_max_for_gg_gb(ns_route.is_inband, &key))
            /* liwh end */
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");
            }
        }

        /* modified by liwh for bug 49771, 2018-11-01 */
        address.prefixlen = 32;
        address.family = key.ip.family;
        sal_memcpy(&address.u, &key.ip.u, sizeof(addr_u));

        if (IPV6_PREFIX_IS_LINK_LOCAL(&address))
        {
            CFG_CONFLICT_RET("Link-local server IPv6 address can't be supported");    
        }

        if (IPV6_PREFIX_IS_MULTICAST(&address))
        {
            CFG_CONFLICT_RET("Multicast server IPv6 address can't be supported");    
        }
        /* liwh end */
    }
    /* liwh end */
    
    /* convert string into flag enum */
    PM_E_RETURN(_ns_route_add_route_convert_str_into_flag_enum(para, argv[1], &flag));

    /* check if the entry exists */
    p_ns_route = tbl_ns_route_get_ns_route(&key);
    if (NULL == p_ns_route) /* create a new entry */
    {
        sal_memcpy(&ns_route.key, &key, sizeof(tbl_ns_route_key_t));
        ns_route.flag = flag;
        ns_route.cnt = 1;

        /* update ns_route_ip table */
        PM_E_RETURN(_ns_route_ip_update_counter(para, ns_route.is_inband, &ns_route.key.ip, TRUE, ns_route.flag));

        /* add entry to db */
        PM_E_RETURN(tbl_ns_route_add_ns_route(&ns_route));
    }
    else /* conflict with other entry */
    {
        if (NS_ROUTE_FLAG_SNMP_SERVER == p_ns_route->flag)
        {
            PM_E_RETURN(_ns_route_ip_update_counter(para, ns_route.is_inband, &p_ns_route->key.ip, TRUE, p_ns_route->flag));

            p_ns_route->cnt++;
            PM_E_RETURN(tbl_ns_route_set_ns_route_field(p_ns_route, TBL_NS_ROUTE_FLD_CNT));

            return rc;
        }
        CFG_INVALID_PARAM_RET("Server already assign to %s", NS_ROUTE_FLAG_TO_STRING(p_ns_route->flag));
    }

    return rc;
}


static int32
_ns_route_delete_route(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_ns_route_key_t key;
    tbl_ns_route_t *p_ns_route;
    uint32 is_inband = 0;

    /* check params */
    if (2 != argc) /* must be $protocol#$remote_port#$remote_ip /$NS_BAND*/
    {
        CFG_INVALID_PARAM_RET("Invalid parameter");
    }

    /* convert params into key */
    PM_E_RETURN(_ns_route_convert_args_into_key(para, argv[0], &key));
    if (0 == sal_strcmp(argv[1], "inband"))
    {
        is_inband = TRUE;
    }
    else
    {
        is_inband = FALSE;
    }
    /* check if the entry exists */
    p_ns_route = tbl_ns_route_get_ns_route(&key);
    if (NULL == p_ns_route) /* create a new entry */
    {
        CFG_INVALID_PARAM_RET("Ns route entry not exists");
    }
    else
    {
        /* mod by hansf for 2018/08/23 contrller ns_route not care is_inband flag */
        if(is_inband!=p_ns_route->is_inband && p_ns_route->flag != NS_ROUTE_FLAG_OPENFLOW_CONTROLLER) 
            // && p_ns_route->flag != NS_ROUTE_FLAG_SNMP_SERVER)
        {
            CFG_INVALID_PARAM_RET("The inform receiver is mismatch");
        }
        /* update counter */
        PM_E_RETURN(_ns_route_ip_update_counter(para, p_ns_route->is_inband, &key.ip, FALSE, p_ns_route->flag));
        if (NS_ROUTE_FLAG_SNMP_SERVER == p_ns_route->flag)
        {
            p_ns_route->cnt--;
            PM_E_RETURN(tbl_ns_route_set_ns_route_field(p_ns_route, TBL_NS_ROUTE_FLD_CNT));
            if (p_ns_route->cnt > 0)
            {
                return rc;
            }
        }
        /* delete entry from db */
        PM_E_RETURN(tbl_ns_route_del_ns_route(&key));
    }

    return rc;
}

/* support sflow by liwh for bug 43981, 2017-06-05 */
int32
ns_route_delete_route(char *input, cfg_cmd_para_t *para)
{
    int32                rc = PM_E_NONE;
    tbl_ns_route_key_t   key;
    tbl_ns_route_t       *p_ns_route;
    
    /* convert params into key */
    PM_E_RETURN(_ns_route_convert_args_into_key(para, input, &key));

    /* check if the entry exists */
    p_ns_route = tbl_ns_route_get_ns_route(&key);
    if (NULL == p_ns_route) /* create a new entry */
    {
        return PM_E_NONE;
    }
    else
    {
        /* update counter */
        PM_E_RETURN(_ns_route_ip_update_counter(para, p_ns_route->is_inband, &key.ip, FALSE, p_ns_route->flag));
        
        /* delete entry from db */
        PM_E_RETURN(tbl_ns_route_del_ns_route(&key));
    }

    return PM_E_NONE;
}
/* liwh end */

/* modified by liwh for bug 45810, 2018-02-06 */
int32
ns_route_check_inband(char *input, cfg_cmd_para_t *para)
{
    int32                rc = PM_E_NONE;
    tbl_ns_route_key_t   key;
    tbl_ns_route_t       *p_ns_route;
    
    /* convert params into key */
    PM_E_RETURN(_ns_route_convert_args_into_key(para, input, &key));

    /* check if the entry exists */
    p_ns_route = tbl_ns_route_get_ns_route(&key);
    if (p_ns_route)
    {
        return p_ns_route->is_inband;
    }

    return PM_E_NOT_EXIST;
}
/* liwh end */

static int32
_ns_route_dump_all(cfg_cmd_para_t *para, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
 
    tbl_ns_route_iterate((TBL_ITER_CB_FUNC)tbl_ns_route_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
ns_route_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    
    CFG_DBG_FUNC();

    switch (para->oper)
    {
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            PM_E_RETURN(_ns_route_dump_all(para, &field));
            break;

        case CDB_OPER_ADD:
            _ns_route_add_route(argv, argc, para);
            break;

        case CDB_OPER_DEL:
            _ns_route_delete_route(argv, argc, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}


static int32 set_newmgmt_snat()
{
    int32 rc = PM_E_NONE;
    char cmd[1024];

    sprintf(cmd, "%s -t nat -A POSTROUTING -o %s --source %s -j MASQUERADE",
            GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX,
            GLB_NS_PORT_FORWARDING_MGMT_IFNAME,
            GLB_NS_ROUTE_DEFAULT_IP_ADDR);
    rc = sal_system(cmd);

    sal_memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "%s -t nat -A POSTROUTING -o %s --source %s -j MASQUERADE",
            GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX_V6,
            GLB_NS_PORT_FORWARDING_MGMT_IFNAME,
            GLB_NS_PORT_FORWARDING_VETH_DEFAULT2_IPV6_ADDR);
    rc = sal_system(cmd);

    return rc;
}


int32
ns_route_start(switch_master_t *master)
{
    set_newmgmt_snat();

    cdb_register_cfg_tbl_cb(TBL_NS_ROUTE, ns_route_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_NS_ROUTE_IP, ns_route_ip_cmd_process);

    return PM_E_NONE;
}


int32
ns_route_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_NS_ROUTE, NULL);
    cdb_register_cfg_tbl_cb(TBL_NS_ROUTE_IP, NULL);

    return PM_E_NONE;
}

