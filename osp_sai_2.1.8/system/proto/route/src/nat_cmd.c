
#include "proto.h"
#include "route.h"
#include "prefix.h"
#include "libnetfilter_conntrack/libnetfilter_conntrack.h"
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_pool_define.h"
#include "gen/tbl_nat_pool.h"
#include "gen/tbl_nat_session_define.h"
#include "gen/tbl_nat_session.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"
#include "gen/tbl_nat_proxy_arp_define.h"
#include "gen/tbl_nat_proxy_arp.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_acl_config_define.h"
#include "gen/tbl_acl_config.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_nat_rtif_inside_define.h"
#include "gen/tbl_nat_rtif_inside.h"
#include "gen/tbl_nat_rtif_outside_define.h"
#include "gen/tbl_nat_rtif_outside.h"
#include "nat_api.h"
#include "nat.h"

/***************************************************************************************************
 * Name         : nat_get_rtif_by_ip
 * Purpose      : get pp_rt_if pointer by addr, except the specified address IP
 * Input        : addr_ipv4_t *addr_ip, addr_ipv4_t *except_addr_ip
 * Output       : tbl_route_if_t **pp_rt_if,  if found, set pp_rt_if
                  uint8 *p_prefixlen
 * Return       : bool: TRUE for found, FALSE for not found.
 * Note         : N/A
***************************************************************************************************/
bool nat_get_rtif_by_ip(addr_ipv4_t *addr_ip, addr_ipv4_t *except_addr_ip, tbl_route_if_t **pp_rt_if, uint8 *p_prefixlen)
{
    ctclib_slistnode_t    *listnode  = NULL;
    ctclib_slistnode_t    *listnode_sub  = NULL;
    ctclib_slistnode_t    *next  = NULL;
    ctclib_slistnode_t    *next_sub  = NULL;
    ds_connected_t        *p_db_conn = NULL;
    tbl_route_if_t        *p_db_rtif = NULL;
    tbl_route_if_master_t *p_master = NULL;
    prefix_t  db_prefix;
    prefix_t  req_prefix;
    uint32    mask = 0;

    p_master = tbl_route_if_get_master();
    if (NULL == p_master)
    {
        SAL_ASSERT(0);
        return FALSE;
    }

    *p_prefixlen = 0;
    *pp_rt_if = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_rtif, listnode, next)
    {
        CTCLIB_SLIST_LOOP_DEL(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode_sub, next_sub)
        {
            if (p_db_conn->flags & RT_IF_ADDR_VIRTUAL)
            {
                /* do not care about VRRP virtual IP.
                   NAT inside interface: admin should guarantee the hosts do not use VIP;
                   NAT outside interface: do not support configure VIP as inside global IP.*/
                continue;
            }

            /* if except_addr_ip not null, check if equal*/
            if (except_addr_ip)
            {
                if (except_addr_ip->s_addr == p_db_conn->key.address.u.prefix4.s_addr)
                {
                    continue;
                }
            }

            /* prepare request data for compare*/
            sal_memset(&req_prefix, 0, sizeof(prefix_t));
            req_prefix.family = AF_INET;
            req_prefix.prefixlen = IPV4_MAX_BITLEN;
            sal_memcpy(&req_prefix.u.prefix4, addr_ip, sizeof(addr_ipv4_t));

            /* prepare db node data*/
            sal_memcpy(&db_prefix, &p_db_conn->key.address, sizeof(prefix_t));

            /* check if same ip*/
            if (req_prefix.u.prefix4.s_addr == db_prefix.u.prefix4.s_addr)
            {
                *p_prefixlen = IPV4_MAX_BITLEN;
                *pp_rt_if = p_db_rtif;
                return TRUE;
            }

            /* check if same subnet*/
            IPV4_LEN_TO_MASK(mask, db_prefix.prefixlen);
            req_prefix.u.prefix4.s_addr = sal_hton32(req_prefix.u.prefix4.s_addr);
            req_prefix.u.prefix4.s_addr = req_prefix.u.prefix4.s_addr & mask;

            db_prefix.u.prefix4.s_addr = sal_hton32(db_prefix.u.prefix4.s_addr);
            db_prefix.u.prefix4.s_addr = db_prefix.u.prefix4.s_addr & mask;
            if (req_prefix.u.prefix4.s_addr == db_prefix.u.prefix4.s_addr)
            {
                *p_prefixlen = db_prefix.prefixlen;
                *pp_rt_if = p_db_rtif;
                /* do not return, maybe next conn has the exactly same ip*/
            }
        }

        /* found same subnet, but no same IP, could return. the other interfaces must be in different subnet.*/
        if (*pp_rt_if)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/***************************************************************************************************
 * Name         : nat_get_rtif_by_iproute
 * Purpose      : get pp_rt_if pointer by addr, find by route
 * Input        : addr_ipv4_t *addr_ip
 * Output       : tbl_route_if_t **pp_rt_if,  if found, set pp_rt_if
                  uint8 *p_prefixlen
 * Return       : bool: TRUE for found, FALSE for not found.
 * Note         : N/A
***************************************************************************************************/
bool nat_get_rtif_by_iproute(addr_ipv4_t *addr_ip, tbl_route_if_t **pp_rt_if, uint8 *p_prefixlen)
{
    tbl_iproute_node_t *p_db_rt_node = NULL;
    tbl_iproute_node_t rt_node;
    addr_ipv4_t dest_tmp;
    int32 index = IPV4_MAX_PREFIXLEN;
    tbl_route_if_t *p_db_rtif = NULL;
    nexthop_key_t   nh_key = {0};

    sal_memset(&rt_node, 0, sizeof(rt_node));
    rt_node.key.p.family = AF_INET;
    
    for(index = IPV4_MAX_PREFIXLEN; index >= 0; index--)
    {
        sal_memcpy(&dest_tmp, addr_ip, sizeof(addr_ipv4_t));
        apply_mask_ipv4_addr(&dest_tmp, index);
        
        rt_node.key.p.prefixlen = index;
        sal_memcpy(&rt_node.key.p.u.prefix4, &dest_tmp, sizeof(addr_ipv4_t));
        p_db_rt_node = tbl_iproute_node_get_iproute_node(&rt_node.key);
        if (p_db_rt_node)
        {
            break;
        }
    }

    if (NULL == p_db_rt_node)
    {
        return FALSE;
    }
    
    /*get nexthop*/
    if (p_db_rt_node->multipath)
    {
        sal_memcpy(&nh_key, &p_db_rt_node->nh_group.nh[0], sizeof(nh_key));
    }
    else
    {
        sal_memcpy(&nh_key, &p_db_rt_node->nh_key, sizeof(nh_key));
    }

    p_db_rtif = tbl_route_if_get_route_if_by_name(nh_key.ifname);
    if (NULL == p_db_rtif)
    {
        return FALSE;
    }

    *pp_rt_if = p_db_rtif;
    *p_prefixlen = index;
    return TRUE;
}

ds_connected_t *
nat_get_rtif_primary_connected_by_ifname(char* if_name)
{
    ctclib_slistnode_t  *listnode   = NULL;
    ds_connected_t      *p_db_conn  = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    tbl_route_if_key_t   rt_if_key;

    sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memcpy(rt_if_key.name, if_name, IFNAME_EXT_SIZE);
    p_db_rtif = tbl_route_if_get_route_if(&rt_if_key);
    if (NULL == p_db_rtif)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
    {
        if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            return p_db_conn;
        }
    }

    return NULL;
}

char *
nat_l3protocol_name(char *str, uint32 len, uint16 l3proto)
{
    str[0] = 0;
    switch (l3proto)
    {
        case IPPROTO_IP:
        {
            sal_snprintf(str, len, "%s", "ip");
            break;
        }
        case IPPROTO_ICMP:
        {
            sal_snprintf(str, len, "%s", "icmp");
            break;
        }
        case IPPROTO_TCP:
        {
            sal_snprintf(str, len, "%s", "tcp");
            break;
        }
        case IPPROTO_UDP:
        {
            sal_snprintf(str, len, "%s", "udp");
            break;
        }
        case IPPROTO_ICMPV6:
        {
            sal_snprintf(str, len, "%s", "icmpv6");
            break;
        }
        default :
        {
            sal_snprintf(str, len, "%u", l3proto);
            break;
        }
    }

    return str;
}


char *
nat_l3protocol_name_upper_case(char *str, uint32 len, uint16 l3proto)
{
    str[0] = 0;
    switch (l3proto)
    {
        case IPPROTO_IP:
        {
            sal_snprintf(str, len, "%s", "IP");
            break;
        }
        case IPPROTO_ICMP:
        {
            sal_snprintf(str, len, "%s", "ICMP");
            break;
        }
        case IPPROTO_TCP:
        {
            sal_snprintf(str, len, "%s", "TCP");
            break;
        }
        case IPPROTO_UDP:
        {
            sal_snprintf(str, len, "%s", "UDP");
            break;
        }
        case IPPROTO_ICMPV6:
        {
            sal_snprintf(str, len, "%s", "ICMPv6");
            break;
        }
        default :
        {
            sal_snprintf(str, len, "%u", l3proto);
            break;
        }
    }

    return str;
}


#if _DESC("nat rule process functions")
/***************************************************************************************************
 * Name         : nat_cmd_rule_parse
 * Purpose      : parse nat rule configuration from CLI.
                  this func is called by rule add and rule del, so p_rule->key must exictly filled here
 * Input        : char **argv, int32 argc, cfg_cmd_para_t *para
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_parse(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    int32 rc = PM_E_NONE;
    int32 val = 0;
    uint8 argc_idx = 0;

    p_rule->key.flags |= GLB_NAT_RULE_FLAG_INSIDE;
    p_rule->key.flags |= GLB_NAT_RULE_FLAG_SNAT;

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("static");
    if (argc_idx != 0xFF)
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_STATIC;
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("access_list");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_ACL;
        sal_strncpy(p_rule->key.acl, argv[argc_idx + 1], ACL_NAME_FULL_NAME_SIZE);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("protocol");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_PROTO;
        if (0 == sal_strcmp(argv[argc_idx + 1], "tcp"))
        {
            p_rule->key.protocol = IPPROTO_TCP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "udp"))
        {
            p_rule->key.protocol = IPPROTO_UDP;
        }else if (0 == sal_strcmp(argv[argc_idx + 1], "icmp"))
        {
            p_rule->key.protocol = IPPROTO_ICMP;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("local_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_IN_LOCAL_IP;
        p_rule->key.in_local_ip.family = AF_INET;
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &p_rule->key.in_local_ip.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("local_port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_IN_LOCAL_PORT;
        PM_CFG_GET_UINT(val, argv[argc_idx + 1], para->p_rs);
        if (val < 65535)
        {
            p_rule->key.in_local_port = val;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("global_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_IN_GLB_IP;
        p_rule->key.in_glb_ip.family = AF_INET;
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &p_rule->key.in_glb_ip.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("outside_if_name");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        char *ifname = NULL;
        char ifname_ext[IFNAME_EXT_SIZE];
        tbl_interface_key_t if_key;
        tbl_interface_t *p_db_if = NULL;

        p_rule->key.flags |= GLB_NAT_RULE_FLAG_OUTSIDE_IF;
        
        ifname = argv[argc_idx + 1];
        IFNAME_ETH2FULL(ifname, ifname_ext);
        
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_strncpy(if_key.name, ifname_ext, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s does not exist.", ifname_ext);
        }
        
        IF_MGR_CHECK_LAG_MEMBER(p_db_if);

        if (GLB_IF_MODE_L3 != p_db_if->mode)
        {
            CFG_CONFLICT_RET("Interface %s is not route interface.", ifname_ext);
        }

        sal_strncpy(p_rule->key.oif, ifname_ext, IFNAME_SIZE);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("pool");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_POOL;
        sal_strncpy(p_rule->key.pool, argv[argc_idx + 1], GLB_NAT_POOL_NAME_LEN);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("global_port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_IN_GLB_PORT;
        PM_CFG_GET_UINT(val, argv[argc_idx + 1], para->p_rs);
        if (val < 65535)
        {
            p_rule->key.in_glb_port = val;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("global_port_range");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        const char *hyphen = "-";
        uint32 hyphen_count = 0;
        uint32 len = 0;
        uint32 i = 0;
        char *p_range = NULL;

        p_rule->key.flags |= GLB_NAT_RULE_FLAG_IN_GLB_PORT;
        p_range = argv[argc_idx + 1];
        len = sal_strlen(p_range);
        for (i = 0; i < len; i++)
        {
            if (p_range[i] == hyphen[0])
            {
                hyphen_count++;
            }
        }
        if (0 == hyphen_count)
        {
            p_rule->key.in_glb_port = sal_atoi(p_range);
            //CHECK_RANGE_VALUE(begin, "Port", 0, 65535, err_desc);
        }
        else if (1 == hyphen_count)
        {
            char *p_tmp = NULL;
            p_rule->key.flags |= GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI;

            val = sal_atoi(argv[argc_idx + 1]);
            if (val < 65535)
            {
                p_rule->key.in_glb_port = val;
            }

            p_tmp = sal_strstr(p_range, hyphen);
            p_tmp++;
            val = sal_atoi(p_tmp);
            if (val < 65535)
            {
                p_rule->key.in_glb_port_hi = val;
            }

            if (p_rule->key.in_glb_port == p_rule->key.in_glb_port_hi)
            {
                p_rule->key.in_glb_port_hi = 0;
                p_rule->key.flags &= (~GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI);
            }
            else if (p_rule->key.in_glb_port > p_rule->key.in_glb_port_hi)
            {
                uint16 tmp = 0;
                tmp = p_rule->key.in_glb_port;
                p_rule->key.in_glb_port = p_rule->key.in_glb_port_hi;
                p_rule->key.in_glb_port_hi = tmp;
            }
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("random");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        p_rule->key.flags |= GLB_NAT_RULE_FLAG_RANDOM;
    }

    return PM_E_NONE;
}


static int32
nat_cmd_rule_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_rule_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_rule_add_check_acl
 * Purpose      : check and convert configuration when add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add_check_acl(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_acl_config_key_t acl_key;
    tbl_acl_config_t *p_db_acl = NULL;

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_ACL)
    {
        sal_memset(&acl_key, 0, sizeof(tbl_acl_config_key_t));
        sal_strncpy(acl_key.name, p_rule->key.acl, ACL_NAME_FULL_NAME_SIZE);
        
        p_db_acl = tbl_acl_config_get_acl_config(&acl_key);
        if (NULL == p_db_acl)
        {
            CFG_INVALID_PARAM_RET("Access group %s does not exist.", acl_key.name);
        }

        if (GLB_ACL_CONFIG_TYPE_NORMAL != p_db_acl->type_identifying)
        {
            CFG_INVALID_PARAM_RET("Type of the access group does not support.");
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_rule_add_check
 * Purpose      : check and convert configuration when add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add_check_in_local_ip(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_route_if_t *p_rt_if = NULL;
    uint8 prefix_len = 0;

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP)
    {
        /* get inside interface*/
        if (TRUE != nat_get_rtif_by_ip(&p_rule->key.in_local_ip.u.prefix4, NULL, &p_rt_if, &prefix_len))
        {
            CFG_INVALID_PARAM_RET("The local IP address should be in the same subnet with one NAT inside interface.");
        }

        if (GLB_IF_NAT_INSIDE != p_rt_if->nat_enable)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not enable NAT inside.", p_rt_if->key.name);
        }

        if (IPV4_MAX_BITLEN == prefix_len)
        {
            CFG_CONFLICT_RET("The local IP address conflicted with interface %s.", p_rt_if->key.name);
        }

        if (TRUE == nat_rule_if_local_addr_port_used(p_rule))
        {
            CFG_CONFLICT_RET("The local IP address and port is configured in another NAT rule.");
        }

        sal_memcpy(p_rule->refer_iif, p_rt_if->key.name, IFNAME_SIZE);
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_rule_add_check
 * Purpose      : check and convert configuration when add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add_check_in_glb_ip(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_route_if_t *p_rt_if = NULL;
    uint8 prefix_len = 0;
    char pool_name[GLB_NAT_POOL_NAME_LEN+1];

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
    {
        /* get outside interface*/
        if (TRUE != nat_get_rtif_by_ip(&p_rule->key.in_glb_ip.u.prefix4, NULL, &p_rt_if, &prefix_len))
        {
            CFG_INVALID_PARAM_RET("The global IP address should be in the same subnet with one NAT outside interface.");
        }

        if (GLB_IF_NAT_OUTSIDE != p_rt_if->nat_enable)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not enable NAT outside.", p_rt_if->key.name);
        }

        if (IPV4_MAX_BITLEN == prefix_len)
        {
            CFG_CONFLICT_RET("The global IP address conflicted with interface %s.", p_rt_if->key.name);
        }

        pool_name[0] = 0;
        if (TRUE == nat_pool_if_addr_used(&p_rule->key.in_glb_ip.u.prefix4, pool_name))
        {
            CFG_CONFLICT_RET("The global IP address conflicted with NAT pool %s.", pool_name);
        }

        if (TRUE == nat_rule_if_global_addr_port_used(p_rule))
        {
            CFG_CONFLICT_RET("The globap IP address and port is configured in another NAT rule.");
        }

        sal_memcpy(p_rule->refer_oif, p_rt_if->key.name, IFNAME_SIZE);
        p_rule->category_id = p_rt_if->nat_category_id;
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_cmd_rule_add_check
 * Purpose      : check and convert configuration when add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add_check_pool(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_nat_pool_key_t pool_key;
    tbl_route_if_key_t rt_if_key;
    tbl_nat_pool_t *p_db_nat_pool = NULL;
    tbl_route_if_t *pst_rt_if = NULL;

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_POOL)
    {
        sal_memset(&pool_key, 0, sizeof(tbl_nat_pool_key_t));
        sal_strncpy(pool_key.name, p_rule->key.pool, GLB_NAT_POOL_NAME_LEN);
        p_db_nat_pool = tbl_nat_pool_get_nat_pool(&pool_key);
        if (NULL == p_db_nat_pool)
        {
            CFG_CONFLICT_RET("Pool %s does not exist", pool_key.name);
        }

        sal_strncpy(rt_if_key.name, p_db_nat_pool->refer_oif, IFNAME_SIZE);
        pst_rt_if  = tbl_route_if_get_route_if(&rt_if_key);
        if (NULL == p_db_nat_pool)
        {
            return PM_E_FAIL;
        }

        sal_memcpy(p_rule->refer_oif, p_db_nat_pool->refer_oif, IFNAME_SIZE);
        p_rule->category_id = pst_rt_if->nat_category_id;
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_rule_add_check_oif
 * Purpose      : check and convert configuration when add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add_check_oif(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_interface_key_t if_key;
    tbl_route_if_key_t rt_if_key;
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t  *p_rt_if = NULL;    

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_memcpy(if_key.name, p_rule->key.oif, GLB_IFNAME_SZ);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (!p_db_if)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not exist.", p_rule->key.oif);
        }

        IF_MGR_CHECK_LAG_MEMBER(p_db_if);

        if (GLB_IF_MODE_L3 != p_db_if->mode)
        {
            CFG_CONFLICT_RET("Interface %s is not route interface.", p_rule->key.oif);
        }

        sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
        sal_memcpy(rt_if_key.name, p_rule->key.oif, GLB_IFNAME_SZ);
        p_rt_if = tbl_route_if_get_route_if(&rt_if_key);
        if (!p_rt_if)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not exist.", p_rule->key.oif);
        }

        if (GLB_IF_NAT_OUTSIDE != p_rt_if->nat_enable)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not enable NAT outside.", p_rule->key.oif);
        }

        if (TRUE == nat_rule_if_outside_rtif_port_used(p_rt_if, p_rule->key.in_glb_port, p_rule->key.in_glb_port_hi))
        {
            CFG_CONFLICT_RET("The outside interface and port conflicted with existing NAT rules.");
        }

        sal_memcpy(p_rule->refer_oif, p_rule->key.oif, GLB_IFNAME_SZ);
     }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_rule_add_check
 * Purpose      : check and convert configuration when add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : tbl_nat_rule_t *p_rule
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add_check_spec(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_nat_rule_master_t *p_rule_master = NULL;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_FAIL;
    }

    if (GLB_NAT_RULE_NUM_MAX == p_rule_master->nat_rule_hash->count)
    {
        CFG_INVALID_PARAM_RET("The total number of NAT rules exceeds the limit %u.", GLB_NAT_RULE_NUM_MAX);
    }
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_rule_add
 * Purpose      : check prara and add rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_add(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    if (tbl_nat_rule_get_nat_rule(&p_rule->key))
    {
        CFG_INVALID_PARAM_RET("The rule exist.");
    }

    if (nat_cmd_rule_add_check_acl(para, p_rule))
    {
        return PM_E_FAIL;
    }
    if (nat_cmd_rule_add_check_in_local_ip(para, p_rule))
    {
        return PM_E_FAIL;
    }

    if (nat_cmd_rule_add_check_in_glb_ip(para, p_rule))
    {
        return PM_E_FAIL;
    }

    if (nat_cmd_rule_add_check_pool(para, p_rule))
    {
        return PM_E_FAIL;
    }

    if (nat_cmd_rule_add_check_oif(para, p_rule))
    {
        return PM_E_FAIL;
    }

    if (nat_cmd_rule_add_check_spec(para, p_rule))
    {
        return PM_E_FAIL;
    }

    return nat_rule_add(p_rule);
}


/***************************************************************************************************
 * Name         : nat_cmd_rule_del
 * Purpose      : check prara and delete rule
 * Input        : cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_rule_del(cfg_cmd_para_t *para, tbl_nat_rule_t *p_rule)
{
    tbl_nat_global_t *p_nat_global;
    tbl_nat_rule_t *p_db_rule = NULL;

    p_nat_global = tbl_nat_global_get_nat_global();
    if (!p_nat_global)
    {
        return PM_E_FAIL;
    }

    p_db_rule = tbl_nat_rule_get_nat_rule(&p_rule->key);
    if (!p_db_rule)
    {
        CFG_INVALID_PARAM_RET("The rule does not exist.");
    }
    
    return nat_rule_del(p_db_rule);
}


int32
nat_cmd_rule_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_nat_rule_t rule;
    cdb_node_t *p_node = NULL;
    field_parse_t field;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_NAT_RULE);
    nat_rule_struct_init(&rule);

    switch (para->oper)
    {
        case CDB_OPER_ADD:
        {
            rc = nat_cmd_rule_parse(argv, argc, para, &rule);
            if (rc)
            {
                return rc;
            }

            rc = nat_cmd_rule_add(para, &rule);
            break;
        }
        case CDB_OPER_DEL:
        {
            rc = nat_cmd_rule_parse(argv, argc, para, &rule);
            if (rc)
            {
                return rc;
            }

            rc = nat_cmd_rule_del(para, &rule);
            break;
        }
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_rule_dump(para, p_node, &field);
            break;
        case CDB_OPER_SET:
            break;
        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}
#endif

#if _DESC("nat pool process functions")


int32
nat_cmd_pool_parse(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_nat_pool_t *p_pool)
{
    int32 rc = PM_E_NONE;
    uint8 argc_idx = 0;
    prefix_t p1, p2;
    uint32 flag = 0;

    if (sal_strlen(argv[0]) > GLB_NAT_POOL_NAME_LEN)
    {
        CFG_INVALID_PARAM_RET("The length of pool name should be less than 32.");
    }

    sal_strncpy(p_pool->key.name, argv[0], GLB_NAT_POOL_NAME_LEN);

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip_low");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_BIT(flag, TBL_NAT_POOL_FLD_IP_LO);
        p_pool->ip_lo.family = AF_INET;
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &p_pool->ip_lo.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip_high");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_BIT(flag, TBL_NAT_POOL_FLD_IP_HI);
        p_pool->ip_hi.family = AF_INET;
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &p_pool->ip_hi.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip_mask");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        GLB_SET_BIT(flag, TBL_NAT_POOL_FLD_IP_MASK);
        p_pool->ip_mask.family = AF_INET;
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &p_pool->ip_mask.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("ip_mask_len");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        uint32 ipv4_mask_len = 0;

        GLB_SET_BIT(flag, TBL_NAT_POOL_FLD_IP_MASK);
        p_pool->ip_mask.family = AF_INET;
        PM_CFG_GET_UINT(ipv4_mask_len, argv[argc_idx + 1], para->p_rs);
        sal_masklen2ip(ipv4_mask_len, &p_pool->ip_mask.u.prefix4);
    }

    if ((GLB_IS_BIT_SET(flag, TBL_NAT_POOL_FLD_IP_LO))
     && (GLB_IS_BIT_SET(flag, TBL_NAT_POOL_FLD_IP_HI))
     && (GLB_IS_BIT_SET(flag, TBL_NAT_POOL_FLD_IP_MASK)))
    {
        /* add pool, must be in same subnet*/
        p1.family = AF_INET;
        IPV4_MASK_TO_LEN(p_pool->ip_mask.u.prefix4.s_addr, p1.prefixlen);
        p1.u.prefix4.s_addr = p_pool->ip_lo.u.prefix4.s_addr;

        p2.family = AF_INET;
        IPV4_MASK_TO_LEN(p_pool->ip_mask.u.prefix4.s_addr, p2.prefixlen);
        p2.u.prefix4.s_addr = p_pool->ip_hi.u.prefix4.s_addr;

        if (TRUE != prefix_compare_net(&p1, &p2)) 
        {
            CFG_INVALID_PARAM_RET("The start IP and end IP should be in the same subnet.");
        }

        if (TRUE == sal_prefix_same(&p1, &p2))
        {
            CFG_INVALID_PARAM_RET("The start IP and end IP should be different.");
        }

        if (sal_hton32(p_pool->ip_lo.u.prefix4.s_addr) > sal_hton32(p_pool->ip_hi.u.prefix4.s_addr))
        {
            uint32 tmp = p_pool->ip_lo.u.prefix4.s_addr;
            p_pool->ip_lo.u.prefix4.s_addr = p_pool->ip_hi.u.prefix4.s_addr;
            p_pool->ip_hi.u.prefix4.s_addr = tmp;
        }
    }

    return PM_E_NONE;
}



static int32
nat_cmd_pool_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_pool_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_pool_add_check_spec
 * Purpose      : check the number of addresses, should be less than 254
 * Input        : cfg_cmd_para_t *para
                  tbl_nat_pool_t *p_pool
 * Output       : char *if_name
 * Return       : PM_E_XX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_pool_add_check_spec(cfg_cmd_para_t *para, tbl_nat_pool_t *p_pool_new)
{
    tbl_nat_pool_master_t *p_pool_master = NULL;

    if (p_pool_new->ip_hi.u.prefix4.s_addr > (p_pool_new->ip_hi.u.prefix4.s_addr + GLB_NAT_POOL_ADDRESS_NUM))
    {
        CFG_INVALID_PARAM_RET("The total number of IP addresses of the pool should be less than %u.", GLB_NAT_POOL_ADDRESS_NUM);
    }

    p_pool_master = tbl_nat_pool_get_master();
    if (!p_pool_master)
    {
        return PM_E_FAIL;
    }

    if (GLB_NAT_POOL_NUM == p_pool_master->nat_pool_hash->count)
    {
        CFG_INVALID_PARAM_RET("The total number of NAT pools exceeds the limit %u.", GLB_NAT_POOL_NUM);
    }
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : nat_cmd_pool_add_check_conflict_with_rule
 * Purpose      : check if the addresses overlapped with any other rule
 * Input        : tbl_nat_pool_t *p_pool
 * Output       : N/A
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_pool_add_check_conflict_with_rule(cfg_cmd_para_t * para, tbl_nat_pool_t *p_pool_new)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_rule_master_t *p_rule_master = NULL;
    tbl_nat_rule_t        *p_db_rule = NULL;
    uint32 ip_val, hi_val, lo_val;

    p_rule_master = tbl_nat_rule_get_master();
    if (!p_rule_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_rule_master->nat_rule_list, p_db_rule, listnode, next)
    {
        if (p_db_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
        {
            ip_val = sal_hton32(p_db_rule->key.in_glb_ip.u.prefix4.s_addr);
            hi_val = sal_hton32(p_pool_new->ip_hi.u.prefix4.s_addr);
            lo_val = sal_hton32(p_pool_new->ip_lo.u.prefix4.s_addr);
            if ((ip_val >= lo_val) && (ip_val <= hi_val))
            {
                CFG_CONFLICT_RET("The pool should not include any global IP configured in other NAT rules.");
            }
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_pool_add_check_overlap
 * Purpose      : check if the addresses overlapped with other pool
 * Input        : cfg_cmd_para_t *para
                  tbl_nat_pool_t *p_pool
 * Output       : char *if_name
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_pool_add_check_overlap(cfg_cmd_para_t *para, tbl_nat_pool_t *p_pool_new)
{
    ctclib_slistnode_t    *listnode    = NULL;
    ctclib_slistnode_t    *next        = NULL;
    tbl_nat_pool_master_t *p_pool_master = NULL;
    tbl_nat_pool_t        *p_db_pool     = NULL;
    uint32   db_hi_val, db_lo_val;
    uint32   hi_val, lo_val;

    p_pool_master = tbl_nat_pool_get_master();
    if (!p_pool_master)
    {
        return PM_E_FAIL;
    }

    lo_val = sal_hton32(p_pool_new->ip_lo.u.prefix4.s_addr);
    hi_val = sal_hton32(p_pool_new->ip_hi.u.prefix4.s_addr);
    CTCLIB_SLIST_LOOP_DEL(p_pool_master->nat_pool_list, p_db_pool, listnode, next)
    {
        db_lo_val = sal_hton32(p_db_pool->ip_lo.u.prefix4.s_addr);
        db_hi_val = sal_hton32(p_db_pool->ip_hi.u.prefix4.s_addr);

        if (((db_lo_val <= hi_val) && (db_hi_val >= hi_val))
         || ((db_lo_val <= lo_val) && (db_hi_val >= lo_val)))
        {
            CFG_INVALID_PARAM_RET("The IP addresses of the pool confilict with pool %s.", p_db_pool->key.name);
        }
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_pool_check_outside_if
 * Purpose      : check if the addresses have the same subnet with one NAT outside if
 * Input        : cfg_cmd_para_t *para
                  tbl_nat_pool_t *p_pool
 * Output       : char *if_name
 * Return       : PM_E_XXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_pool_add_check_outside_if(cfg_cmd_para_t *para, tbl_nat_pool_t *p_pool, char *if_name)
{
    int32 ret = PM_E_NONE;
    
    ret = nat_pool_get_outside_rtif(p_pool, NULL, if_name);

    if (PM_E_CFG_CONFLICT == ret)
    {
        CFG_INVALID_PARAM_RET("The IP addresses of the pool confilict with interface %s.", if_name);
    }

    if (PM_E_NOT_EXIST == ret)
    {
        /* not found*/
        CFG_INVALID_PARAM_RET("The IP addresses of the pool should be in the same subnet with one NAT outside interface.");
    }

    return ret;
}

int32
nat_cmd_pool_add(cfg_cmd_para_t * para, tbl_nat_pool_t *p_pool)
{
    /* 1.  check para*/
    /* 1.1  check if exist*/
    if (tbl_nat_pool_get_nat_pool(&p_pool->key))
    {
        CFG_INVALID_PARAM_RET("The pool exist.");
    }

    /* 1.1  check if confilict with rule inside global global IP*/
    if (nat_cmd_pool_add_check_conflict_with_rule(para, p_pool))
    {
        return PM_E_FAIL;
    }

    /* 1.2  check spec*/
    if (nat_cmd_pool_add_check_spec(para, p_pool))
    {
        return PM_E_FAIL;
    }

    /* 1.3 check if the addressed overlapped with other pool*/
    if (nat_cmd_pool_add_check_overlap(para, p_pool))
    {
        return PM_E_FAIL;
    }

    /* 1.4 check if the addresses should have the same subnet with one NAT outside if*/
    if (nat_cmd_pool_add_check_outside_if(para, p_pool, p_pool->refer_oif))
    {
        return PM_E_FAIL;
    }

    /* 2.  check para*/
    return nat_pool_add(p_pool);
}

int32
nat_cmd_pool_del(cfg_cmd_para_t * para, tbl_nat_pool_t *p_pool)
{
    tbl_nat_pool_t * p_db_pool = NULL;

    /* 1. check para*/
    p_db_pool = tbl_nat_pool_get_nat_pool(&p_pool->key);
    if (!p_db_pool)
    {
        if (para)
        {
            CFG_INVALID_PARAM_RET("The pool does not exist.");
        }
    }

    para->p_rs->rc = PM_E_CFG_PROMPT;
    sal_snprintf(para->p_rs->desc, 64, "NAT rules refer to this pool will be automatitically deletd.");

    /* 2. call process function*/
    return nat_pool_del(p_db_pool);
}


int32
nat_cmd_pool_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_nat_pool_t pool;
    cdb_node_t *p_node = NULL;
    field_parse_t field;

    CFG_DBG_FUNC();

    nat_pool_struct_init(&pool);
    p_node = cdb_get_tbl(TBL_NAT_POOL);
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            rc = nat_cmd_pool_parse(argv, argc, para, &pool);
            if (rc)
            {
                return rc;
            }
        
            rc = nat_cmd_pool_add(para, &pool);
            break;

        case CDB_OPER_SET:
            break;

        case CDB_OPER_DEL:
            rc = nat_cmd_pool_parse(argv, argc, para, &pool);
            if (rc)
            {
                return rc;
            }
        
            rc = nat_cmd_pool_del(para, &pool);
            break;

        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_pool_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}
#endif

#if _DESC("nat global configurations process")
static int32
nat_cmd_global_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_global_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
nat_cmd_global_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 field_id = 0;
    uint32 value = 0;
    cdb_node_t *p_node = NULL;
    tbl_nat_global_t *p_db_nat_global;
    cdb_field_t *p_field = NULL;
    field_parse_t field;
    uint32 curr_cnt = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_NAT_GLOBAL);
    p_db_nat_global = tbl_nat_global_get_nat_global();
    switch (para->oper)
    {
        case CDB_OPER_ADD:
            break;
        case CDB_OPER_SET:
        {
            PM_CFG_CHECK_MIN_PARAMS(argc, 2, para->p_rs);
            
            field_id = cdb_get_field_id(argv[0], p_node, &p_field);
            if (field_id < 0)
            {
                CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
            }

            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }

            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            switch (field_id)
            {
                case TBL_NAT_GLOBAL_FLD_SESSION_LIMIT:
                {
                    if (0 == value)
                    {
                        value = NAT_SESSION_SPEC;
                    }
                    p_db_nat_global->session_limit = value;

                    curr_cnt = p_db_nat_global->session_cnt_upstream + p_db_nat_global->session_cnt_dnstream;
                    if (p_db_nat_global->session_limit > curr_cnt)
                    {
                        nat_nft_connlimit_set_global(p_db_nat_global->session_limit);
                    }
                    break;
                }
                case TBL_NAT_GLOBAL_FLD_TIMEOUT:
                {
                    if (0 == value)
                    {
                        value = NAT_SESSION_TTL_DEFAULT;
                    }

                    p_db_nat_global->timeout = value;
                    break;
                }
                case TBL_NAT_GLOBAL_FLD_TIMEOUT_ICMP:
                {
                    p_db_nat_global->timeout_icmp = value;
                    break;
                }
                case TBL_NAT_GLOBAL_FLD_TIMEOUT_TCP:
                {
                    p_db_nat_global->timeout_tcp = value;
                    break;
                }
                case TBL_NAT_GLOBAL_FLD_TIMEOUT_UDP:
                {
                    p_db_nat_global->timeout_udp = value;
                    break;
                }
                default:
                    break;
            }
            /* modify ttl configuration, do not refresh existed sessions of this host, wait for aging. or clear sessions manually*/
            tbl_nat_global_set_nat_global_field(p_db_nat_global, field_id);
            break;
        }
        case CDB_OPER_DEL:
            break;
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = nat_cmd_global_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}
#endif

#if _DESC("nat session configuration process")
static int32
nat_cmd_session_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_session_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
nat_cmd_session_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    cdb_node_t *p_node = NULL;
    field_parse_t field;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_NAT_SESSION);

    switch (para->oper)
    {
        case CDB_OPER_ADD:
        {
            break;
        }
        case CDB_OPER_SET:
        {
            break;
        }
        case CDB_OPER_DEL:
        {
            break;
        }
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_session_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}
#endif

#if _DESC("nat translations show and clear, include dynamic NAT sessions and static 1:1 IP mapping NAT rules.")
/***************************************************************************************************
 * Name         : nat_cmd_translations_filter_session
 * Purpose      : filter session by translation filter
 * Input        : tbl_nat_session_t *p_session, nat_translations_show_filter_s *p_filter
 * Output       : N/A
 * Return       : TRUE filtered, FALSE not filtered
 * Note         : N/A
***************************************************************************************************/
bool
nat_cmd_translations_filter_session(tbl_nat_session_t *p_session, nat_translations_show_filter_s *p_filter)
{
    if ((p_filter->ip_proto != 0) && (p_session->key.ip_protocol != p_filter->ip_proto))
    {
        return TRUE;
    }

    if ((IPPROTO_TCP == p_session->key.ip_protocol) || (IPPROTO_UDP == p_session->key.ip_protocol))
    {
        if ((p_filter->local_port != 0) && (p_session->key.proto_info.l4_port.inside_local != p_filter->local_port))
        {
            return TRUE;
        }
    
        if ((p_filter->global_port != 0) && (p_session->key.proto_info.l4_port.inside_global != p_filter->global_port))
        {
            return TRUE;
        }
    }
    else if ((p_filter->local_port != 0) || (p_filter->global_port != 0)) /* not TCP or UDP, but Port filter set*/
    {
        return TRUE;
    }

    if ((p_filter->local_ip.u.prefix4.s_addr != 0)
     && (0 != sal_memcmp(&p_session->key.inside_local_ip.u.prefix4, &p_filter->local_ip.u.prefix4, sizeof(addr_ipv4_t))))
    {
        return TRUE;
    }
    
    if ((p_filter->global_ip.u.prefix4.s_addr != 0)
     && (0 != sal_memcmp(&p_session->key.inside_global_ip.u.prefix4, &p_filter->global_ip.u.prefix4, sizeof(addr_ipv4_t))))
    {
        return TRUE;
    }

    return FALSE;
}

/***************************************************************************************************
 * Name         : nat_cmd_translations_filter_rule
 * Purpose      : filter rule by translation filter
 * Input        : tbl_nat_rule_t *p_rule, nat_translations_show_filter_s *p_filter
 * Output       : N/A
 * Return       : TRUE filtered, FALSE not filtered
 * Note         : N/A
***************************************************************************************************/
bool
nat_cmd_translations_filter_rule(tbl_nat_rule_t *p_rule, nat_translations_show_filter_s *p_filter)
{
    if (!(p_rule->key.flags & GLB_NAT_RULE_FLAG_STATIC))
    {
        return TRUE;
    }
    
    if (!(p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_IP))
    {
        return TRUE;
    }
    
    if ((p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_LOCAL_PORT)
     || (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT)
     || (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_PORT_HI))
    {
        return TRUE;
    }

    if ((0 != p_filter->ip_proto)
     || (0 != p_filter->local_port)
     || (0 != p_filter->global_port))
    {
        return TRUE;
    }
    
    if ((p_filter->local_ip.u.prefix4.s_addr != 0)
     && (0 != sal_memcmp(&p_rule->key.in_local_ip.u.prefix4, &p_filter->local_ip.u.prefix4, sizeof(addr_ipv4_t))))
    {
        return TRUE;
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
    {
        if ((p_filter->global_ip.u.prefix4.s_addr != 0)
         && (0 != sal_memcmp(&p_rule->key.in_glb_ip.u.prefix4, &p_filter->global_ip.u.prefix4, sizeof(addr_ipv4_t))))
        {
            return TRUE;
        }
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        ds_connected_t  *ifc_primary = NULL;
    
        ifc_primary = nat_get_rtif_primary_connected_by_ifname(p_rule->key.oif);
        if (ifc_primary)
        {
            if ((p_filter->global_ip.u.prefix4.s_addr != 0)
         && (0 != sal_memcmp(&p_rule->key.in_glb_ip.u.prefix4, &p_filter->global_ip.u.prefix4, sizeof(addr_ipv4_t))))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

int32
nat_cmd_translations_show_detail_session_one(FILE *fp, tbl_nat_session_t *p_session, uint32 showcnt)
{
    char protocol[16];
    char addr[64];
    char str[256];
    char time_c[64];
    char if_inside[IFNAME_SIZE];
    char if_outside[IFNAME_SIZE];
    sal_time_t createtime;
    sal_time_t boottime;
    sal_time_t livetime;

    if (showcnt)
    {
        /* not first one, append \n to file*/
        sal_fprintf(fp, "\n");
    }

    protocol[0] = 0;
    sal_fprintf(fp, "%-4s", nat_l3protocol_name_upper_case(protocol, 16, p_session->key.ip_protocol));

    if ((IPPROTO_TCP == p_session->key.ip_protocol) || (IPPROTO_UDP == p_session->key.ip_protocol))
    {
        str[0] = 0;
        sprintf(str, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_local_ip.u.prefix4), p_session->key.proto_info.l4_port.inside_local);
        sal_fprintf(fp, " %-21s", str);

        str[0] = 0;
        sprintf(str, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_global_ip.u.prefix4), p_session->key.proto_info.l4_port.inside_global);
        sal_fprintf(fp, " %-21s", str);

        str[0] = 0;
        sprintf(str, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.outside_global_ip.u.prefix4), p_session->key.proto_info.l4_port.outside_global);
        sal_fprintf(fp, " %-21s", str);
        sal_fprintf(fp, "\n");

        if (0 != p_session->helper[0])
        {
            sal_fprintf(fp, " ALG:%s\n", p_session->helper);
        }
    }
    else if (IPPROTO_ICMP == p_session->key.ip_protocol)
    {
        str[0] = 0;
        sprintf(str, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_local_ip.u.prefix4), p_session->key.proto_info.icmp.ID);
        sal_fprintf(fp, " %-21s", str);
        
        str[0] = 0;
        sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_global_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", str);
        
        str[0] = 0;
        sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.outside_global_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", str);
        sal_fprintf(fp, "\n");

        sal_fprintf(fp, " Type:%u, Code:%u, ID:%u\n", p_session->key.proto_info.icmp.type, 
                                                           p_session->key.proto_info.icmp.code, 
                                                           p_session->key.proto_info.icmp.ID);
    }
    else
    {
        str[0] = 0;
        sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_local_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", str);
        
        str[0] = 0;
        sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_global_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", str);
        
        str[0] = 0;
        sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.outside_global_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", str);
        sal_fprintf(fp, "\n");
    }

    boottime = ctc_time_boottime_sec(NULL);
    livetime = boottime - p_session->timestamp;

    /* get create time string*/
    createtime = sal_time(NULL);
    createtime = createtime - livetime;
    sal_time_strf(time_c, 64, "%T %m/%d/%Y", sal_localtime(&createtime));

    sal_fprintf(fp, " Create:%s,", time_c);
    /* get if inside and outside*/
    if (PM_E_NONE == tbl_interface_get_name_by_ifindex(p_session->iif_index, if_inside, IFNAME_SIZE))
    {
        sal_fprintf(fp, " Inside:%s,", if_inside);
    }
    else
    {
        sal_fprintf(fp, " Inside:%s,", "-");
    }

    if (PM_E_NONE == tbl_interface_get_name_by_ifindex(p_session->oif_index, if_outside, IFNAME_SIZE))
    {
        sal_fprintf(fp, " Outside:%s", if_outside);
    }
    else
    {
        sal_fprintf(fp, " Outside:%s", "-");
    }

    sal_fprintf(fp, "\n");
    return PM_E_NONE;
}
int32
nat_cmd_translations_show_detail_rule_one(FILE *fp, tbl_nat_rule_t *p_rule, uint32 showcnt)
{
    char addr[64];
    char str[256];
    char protocol[16];

    if (showcnt)
    {
        /* not first one, append \n to file*/
        sal_fprintf(fp, "\n");
    }

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
    {
        protocol[0] = 0;
        sal_fprintf(fp, "%-4s", nat_l3protocol_name_upper_case(protocol, 16, p_rule->key.protocol));
    }
    else
    {
        sal_fprintf(fp, "%-4s", "-");
    }

    str[0] = 0;
    sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_rule->key.in_local_ip.u.prefix4), "-");
    sal_fprintf(fp, " %-21s", str);

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
    {
        str[0] = 0;
        sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_rule->key.in_glb_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", str);
    }
    else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        ds_connected_t  *ifc_primary = NULL;
    
        ifc_primary = nat_get_rtif_primary_connected_by_ifname(p_rule->key.oif);
        if (ifc_primary)
        {
            str[0] = 0;
            sprintf(str, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &ifc_primary->key.address.u.prefix4), "-");
            sal_fprintf(fp, " %-21s", str);
        }
    }

    str[0] = 0;
    sprintf(str, "%s:%s","-", "-");
    sal_fprintf(fp, " %-21s", str);
    sal_fprintf(fp, "\n");

    /* print string*/
    sal_fprintf(fp, " Create: %s, Inside: %s, Outside: %s", "-", "-", "-");
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
nat_cmd_translations_show_detail(FILE *fp, nat_translations_show_filter_s *p_filter)
{
    uint32 show_count = 0;
    tbl_nat_session_master_t *p_session_master = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    ctclib_slistnode_t       *next = NULL;
    tbl_nat_session_t        *p_session = NULL;

    p_session_master = tbl_nat_session_get_master();
    if (!p_session_master)
    {
        return PM_E_FAIL;    
    }
    
    sal_fprintf(fp, "Pro  Inside local          Inside global         Outside global       \n");
    sal_fprintf(fp, "----+---------------------+---------------------+---------------------\n");

    CTCLIB_SLIST_LOOP_DEL(p_session_master->nat_session_list, p_session, listnode, next)
    {
        if (TRUE == nat_cmd_translations_filter_session(p_session, p_filter))
        {
            continue;
        }

        nat_cmd_translations_show_detail_session_one(fp, p_session, show_count);
        show_count++;
    } 
    if (0 == show_count)
    {
        sal_fprintf(fp, "%-4s %-21s %-21s %-21s\n", "-", "-", "-", "-");
    }
    sal_fprintf(fp, "----+---------------------+---------------------+---------------------\n");
    sal_fprintf(fp, "Total: %u\n", show_count);
    return PM_E_NONE;
}

int32
nat_cmd_translations_show_simple_session_one(FILE *fp, tbl_nat_session_t *p_session, uint32 showcnt)
{
    char addr[64];
    char sz_inside[256];
    char sz_outside[256];
    char protocol[16];

    protocol[0] = 0;
    sal_fprintf(fp, "%-4s", nat_l3protocol_name_upper_case(protocol, 16, p_session->key.ip_protocol));

    if ((IPPROTO_TCP == p_session->key.ip_protocol) || (IPPROTO_UDP == p_session->key.ip_protocol))
    {
        sz_inside[0] = 0;
        sprintf(sz_inside, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_local_ip.u.prefix4), p_session->key.proto_info.l4_port.inside_local);
        sal_fprintf(fp, " %-21s", sz_inside);

        sz_outside[0] = 0;
        sprintf(sz_outside, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_global_ip.u.prefix4), p_session->key.proto_info.l4_port.inside_global);
        sal_fprintf(fp, " %-21s", sz_outside);
    }
    else if (IPPROTO_ICMP == p_session->key.ip_protocol)
    {
        sz_inside[0] = 0;
        sprintf(sz_inside, "%s:%u",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_local_ip.u.prefix4), p_session->key.proto_info.icmp.ID);
        sal_fprintf(fp, " %-21s", sz_inside);
        
        sz_outside[0] = 0;
        sprintf(sz_outside, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_global_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", sz_outside);
    }
    else
    {
        sz_inside[0] = 0;
        sprintf(sz_inside, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_local_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", sz_inside);
        
        sz_outside[0] = 0;
        sprintf(sz_outside, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_session->key.inside_global_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", sz_outside);
    }

    sal_fprintf(fp, "\n");
    return PM_E_NONE;
}

int32
nat_cmd_translations_show_simple_rule_one(FILE *fp, tbl_nat_rule_t *p_rule)
{
    char addr[64];
    char sz_inside[256];
    char sz_outside[256];
    char protocol[16];

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_PROTO)
    {
        protocol[0] = 0;
        sal_fprintf(fp, "%-4s", nat_l3protocol_name_upper_case(protocol, 16, p_rule->key.protocol));
    }
    else
    {
        sal_fprintf(fp, "%-4s", "-");
    }

    sz_inside[0] = 0;
    sprintf(sz_inside, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_rule->key.in_local_ip.u.prefix4), "-");
    sal_fprintf(fp, " %-21s", sz_inside);

    if (p_rule->key.flags & GLB_NAT_RULE_FLAG_IN_GLB_IP)
    {
        sz_outside[0] = 0;
        sprintf(sz_outside, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &p_rule->key.in_glb_ip.u.prefix4), "-");
        sal_fprintf(fp, " %-21s", sz_outside);
    }
    else if (p_rule->key.flags & GLB_NAT_RULE_FLAG_OUTSIDE_IF)
    {
        ds_connected_t  *ifc_primary = NULL;
    
        ifc_primary = nat_get_rtif_primary_connected_by_ifname(p_rule->key.oif);
        if (ifc_primary)
        {
            sz_outside[0] = 0;
            sprintf(sz_outside, "%s:%s",cdb_addr_ipv4_val2str(addr, 64, &ifc_primary->key.address.u.prefix4), "-");
            sal_fprintf(fp, " %-21s", sz_outside);
        }
    }

    sal_fprintf(fp, "\n");
    return PM_E_NONE;
}

int32
nat_cmd_translations_show_simple(FILE *fp, nat_translations_show_filter_s *p_filter)
{
    uint32 show_count = 0;
    tbl_nat_session_master_t *p_session_master = NULL;
    ctclib_slistnode_t       *listnode = NULL;
    ctclib_slistnode_t       *next = NULL;
    tbl_nat_session_t        *p_session = NULL;

    p_session_master = tbl_nat_session_get_master();
    if (!p_session_master)
    {
        return PM_E_FAIL;    
    }
    
    sal_fprintf(fp, "Pro  Inside local          Inside global         \n");
    sal_fprintf(fp, "----+---------------------+----------------------\n");

    CTCLIB_SLIST_LOOP_DEL(p_session_master->nat_session_list, p_session, listnode, next)
    {
        if (TRUE == nat_cmd_translations_filter_session(p_session, p_filter))
        {
            continue;
        }

        nat_cmd_translations_show_simple_session_one(fp, p_session, show_count);
        show_count++;
    } 
    if (0 == show_count)
    {
        sal_fprintf(fp, "%-4s %-21s %-21s\n", "-", "-", "-");
    }
    sal_fprintf(fp, "----+---------------------+----------------------\n");
    sal_fprintf(fp, "Total: %u\n", show_count);
    return PM_E_NONE;
}

/*Add by zhush for bug 52916, 2019-07-26*/
int32
nat_cmd_translations_show_summary(FILE *fp)
{
    tbl_nat_global_t                *p_nat_global               = NULL;
    tbl_nat_pool_master_t           *p_nat_pool_master          = NULL;
    tbl_nat_rule_master_t           *p_nat_rule_master          = NULL;
    tbl_nat_session_limit_master_t  *p_nat_session_limit_master = NULL;
    tbl_nat_rtif_inside_master_t    *p_nat_rtif_inside_master   = NULL;
    tbl_nat_rtif_outside_master_t   *p_nat_rtif_outside_master  = NULL;

    p_nat_global               = tbl_nat_global_get_nat_global();
    p_nat_pool_master          = tbl_nat_pool_get_master();
    p_nat_rule_master          = tbl_nat_rule_get_master();
    p_nat_session_limit_master = tbl_nat_session_limit_get_master();
    p_nat_rtif_inside_master   = tbl_nat_rtif_inside_get_master();
    p_nat_rtif_outside_master  = tbl_nat_rtif_outside_get_master();
    
    if (!p_nat_global || 
        !p_nat_pool_master || 
        !p_nat_rule_master || 
        !p_nat_session_limit_master || 
        !p_nat_rtif_inside_master || 
        !p_nat_rtif_outside_master)
    {
        return PM_E_FAIL;    
    }
    
    sal_fprintf(fp, "NAT pool used number is: %d\n", p_nat_pool_master->nat_pool_hash->count);
    sal_fprintf(fp, "NAT rule added number is: %d\n", p_nat_rule_master->nat_rule_hash->count);
    sal_fprintf(fp, "NAT inside interface number is: %d\n", p_nat_rtif_inside_master->nat_rtif_inside_hash->count);
    sal_fprintf(fp, "NAT outside interface number is: %d\n", p_nat_rtif_outside_master->nat_rtif_outside_hash->count);
    sal_fprintf(fp, "NAT translations limit node number is: %d\n", p_nat_session_limit_master->nat_session_limit_hash->count);
    sal_fprintf(fp, "NAT upstream translation number is: %d\n", p_nat_global->session_cnt_upstream);
    sal_fprintf(fp, "NAT downstream translation number is: %d\n", p_nat_global->session_cnt_dnstream);
    
    return PM_E_NONE;
}
/*end zhush*/

/***************************************************************************************************
 * Name         : nat_cmd_translations_show
 * Purpose      : show translations
 * Input        : char **argv, int32 argc, cfg_cmd_para_t *para
 * Output       : N/A
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_translations_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint8 argc_idx = 0;
    FILE  *fp = NULL;
    int32 oper = 0;
    nat_translations_show_filter_s filter;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&filter, 0, sizeof(nat_translations_show_filter_s));
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("protocol");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        if (0 == sal_strcmp(argv[argc_idx + 1], "icmp"))
        {
            filter.ip_proto = IPPROTO_ICMP;
        }
        if (0 == sal_strcmp(argv[argc_idx + 1], "tcp"))
        {
            filter.ip_proto = IPPROTO_TCP;
        }
        else if (0 == sal_strcmp(argv[argc_idx + 1], "udp"))
        {
            filter.ip_proto = IPPROTO_UDP;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("local_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &filter.local_ip.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("local_port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        uint32 val = 0;
        PM_CFG_GET_UINT(val, argv[argc_idx + 1], para->p_rs);
        if (val < 65535)
        {
            filter.local_port = val;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("global_ip");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &filter.global_ip.u.prefix4, para->p_rs);
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("global_port");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        uint32 val = 0;
        PM_CFG_GET_UINT(val, argv[argc_idx + 1], para->p_rs);
        if (val < 65535)
        {
            filter.global_port = val;
        }
    }

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("detail");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        oper = NAT_SHOW_TRANS_OPER_DETAIL;
    }

/*Add by zhush for bug 52916, 2019-07-26*/
    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("summary");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        oper = NAT_SHOW_TRANS_OPER_SUMMARY;
    }
    
    switch(oper)
    {
        case NAT_SHOW_TRANS_OPER_DETAIL:
            nat_cmd_translations_show_detail(fp, &filter);
            break;
            
        case NAT_SHOW_TRANS_OPER_SUMMARY:  
            nat_cmd_translations_show_summary(fp);
            break;
        default:
            nat_cmd_translations_show_simple(fp, &filter);
            break;
    }
/*end zhush*/

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : nat_cmd_translations_clear
 * Purpose      : clear translations
 * Input        : char **argv, int32 argc, cfg_cmd_para_t *para
 * Output       : N/A
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
nat_cmd_translations_clear(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    //int32 rc = PM_E_NONE;
    uint8 argc_idx = 0;
    nat_nfct_hook_s hook_para;
    tbl_nat_global_t *p_nat_global = NULL;

    argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("outside_if_name");
    if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
    {
        char *ifname = NULL;
        char ifname_ext[IFNAME_EXT_SIZE];
        tbl_route_if_key_t   rt_if_key;
        tbl_interface_key_t  if_key;
        tbl_route_if_t      *p_db_rt_if = NULL;
        tbl_interface_t     *p_db_if = NULL;

        ifname = argv[argc_idx + 1];
        IFNAME_ETH2FULL(ifname, ifname_ext);

        sal_memset(&if_key, 0, sizeof(if_key));
        sal_memcpy(if_key.name, ifname_ext, IFNAME_SIZE);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (!p_db_if)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not exist.", ifname_ext);
        }

        if (GLB_IF_MODE_L3 != p_db_if->mode)
        {
            CFG_CONFLICT_RET("Interface %s is not route interface.", ifname_ext);
        }

        sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
        sal_strncpy(rt_if_key.name, ifname_ext, IFNAME_SIZE);
        p_db_rt_if = tbl_route_if_get_route_if(&rt_if_key);
        if (NULL == p_db_rt_if)
        {
            CFG_INVALID_PARAM_RET("Interface %s does not exist.", ifname_ext);
        }

        /* clear session by oif*/
        nat_session_clear_by_oif(p_db_rt_if);

        /* clear kernel ALG session by oif*/
        sal_memset(&hook_para, 0, sizeof(nat_nfct_hook_s));
        hook_para.argv[0] = p_db_rt_if;
        nat_nfct_scan(nat_nfct_hook_clear_session, &hook_para);
    }
    else
    {
        /* clear session all*/
        nat_session_clear_all();

        /* clear kernel ALG session*/
        sal_memset(&hook_para, 0, sizeof(nat_nfct_hook_s));
        hook_para.argv[0] = NULL;
        nat_nfct_scan(nat_nfct_hook_clear_session, &hook_para);

        p_nat_global = tbl_nat_global_get_nat_global();
        if (!p_nat_global)
        {
            return PM_E_FAIL;    
        }

        p_nat_global->nfct_kernel_notify_add_cnt = 0;
        p_nat_global->nfct_kernel_notify_del_cnt = 0;
        p_nat_global->nfct_notify_kernel_add_cnt = 0;
        p_nat_global->nfct_notify_kernel_del_cnt = 0;
        p_nat_global->session_aging_del_cnt = 0;
        p_nat_global->session_aging_upd_ttl_cnt = 0;
    }

    return PM_E_NONE;
}
#endif

#if _DESC("nat session limit configuration process")
static int32
nat_cmd_session_limit_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_session_limit_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
nat_cmd_session_limit_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 curr_cnt = 0;
    int32 rc = PM_E_NONE;
    cdb_node_t *p_node = NULL;
    field_parse_t field;
    uint8 argc_idx = 0;
    tbl_nat_session_limit_t session_limit;
    tbl_nat_session_limit_t *p_db_session_limit = NULL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_NAT_SESSION_LIMIT);

    switch (para->oper)
    {
        case CDB_OPER_ADD:
        {
            argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("host");
            if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
            {
                sal_memset(&session_limit, 0, sizeof(session_limit));
            
                session_limit.key.family = AF_INET;
                CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &session_limit.key.u.prefix4, para->p_rs);
            
                PM_CFG_GET_UINT(session_limit.limit, argv[argc_idx + 2], para->p_rs);
                p_db_session_limit = tbl_nat_session_limit_get_nat_session_limit(&session_limit.key);
                if (p_db_session_limit)
                {
                    p_db_session_limit->limit = session_limit.limit;

                    curr_cnt = p_db_session_limit->count_upstream + p_db_session_limit->count_dnstream;
                    if (p_db_session_limit->limit > curr_cnt)
                    {
                        nat_nft_connlimit_set(p_db_session_limit, p_db_session_limit->limit);
                    }

                    tbl_nat_session_limit_set_nat_session_limit_field(p_db_session_limit, TBL_NAT_SESSION_LIMIT_FLD_LIMIT);
                    return PM_E_NONE;
                }

                if (NAT_SESSION_SPEC != session_limit.limit)
                {
                    tbl_nat_session_limit_master_t *p_master = NULL;

                    p_master = tbl_nat_session_limit_get_master();
                    if (GLB_NAT_SESSION_LIMIT_NODE_NUM_MAX == p_master->nat_session_limit_hash->count)
                    {
                        CFG_INVALID_PARAM_RET("The total number of NAT session limit nodes exceeds the limit %u.", GLB_NAT_SESSION_LIMIT_NODE_NUM_MAX);
                    }

                    nat_nft_connlimit_set(&session_limit, session_limit.limit);
                    tbl_nat_session_limit_add_nat_session_limit(&session_limit);
                }

                /* modify limit configuration, do not refresh existed sessions of this host, wait for aging. or clear sessions manually*/
            }
            break;
        }
        case CDB_OPER_DEL:
        {
            argc_idx = CFG_GET_ARGC_INDEX_ENHANCE("host");
            if (argc_idx != 0xFF && 0 != sal_strlen(argv[argc_idx + 1]))
            {
                sal_memset(&session_limit, 0, sizeof(session_limit));

                session_limit.key.family = AF_INET;
                CFG_PARSER_IP_ADDR(argv[argc_idx + 1], &session_limit.key.u.prefix4, para->p_rs);

                p_db_session_limit = tbl_nat_session_limit_get_nat_session_limit(&session_limit.key);
                if (NULL == p_db_session_limit)
                {
                    return PM_E_NONE;
                }

                nat_nft_connlimit_unset(p_db_session_limit);
                tbl_nat_session_limit_del_nat_session_limit(&session_limit.key);
            }
            break;
        }
        case CDB_OPER_SET:
            break;
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_session_limit_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}
#endif

#if _DESC("nat proxy arp configuration process")
static int32
nat_cmd_proxy_arp_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_proxy_arp_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}


int32
nat_cmd_proxy_arp_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    cdb_node_t *p_node = NULL;
    field_parse_t field;
    //uint8 argc_idx = 0;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_NAT_PROXY_ARP);

    switch (para->oper)
    {
        case CDB_OPER_ADD:
        case CDB_OPER_DEL:
        case CDB_OPER_SET:
            break;
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_proxy_arp_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}
#endif

static int32
nat_cmd_rtif_inside_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_rtif_inside_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
nat_cmd_rtif_inside_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    cdb_node_t *p_node = NULL;
    field_parse_t field;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_NAT_RTIF_INSIDE);

    switch (para->oper)
    {
        case CDB_OPER_ADD:
        case CDB_OPER_DEL:
        case CDB_OPER_SET:
            break;
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_rtif_inside_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}

static int32
nat_cmd_rtif_outside_dump(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_nat_rtif_outside_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
nat_cmd_rtif_outside_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    cdb_node_t *p_node = NULL;
    field_parse_t field;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_NAT_RTIF_OUTSIDE);

    switch (para->oper)
    {
        case CDB_OPER_ADD:
        case CDB_OPER_DEL:
        case CDB_OPER_SET:
            break;
        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            
            rc = nat_cmd_rtif_outside_dump(para, p_node, &field);
            break;

        case CDB_OPER_SHOW:
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}

/***************************************************************************************************
 * Name         : nat_cmd_init
 * Purpose      : cmd init
 * Input        : N/A
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
void
nat_cmd_init()
{
    cdb_register_cfg_tbl_cb(TBL_NAT_GLOBAL, nat_cmd_global_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_RULE, nat_cmd_rule_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_POOL, nat_cmd_pool_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_SESSION, nat_cmd_session_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_SESSION_LIMIT, nat_cmd_session_limit_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_PROXY_ARP, nat_cmd_proxy_arp_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_RTIF_INSIDE, nat_cmd_rtif_inside_process);
    cdb_register_cfg_tbl_cb(TBL_NAT_RTIF_OUTSIDE, nat_cmd_rtif_outside_process);

    cdb_register_cfg_act_cb(ACT_SHOW_NAT_SESSION, nat_cmd_translations_show);
    cdb_register_cfg_act_cb(ACT_CLEAR_NAT_SESSION, nat_cmd_translations_clear);
    return;
}

