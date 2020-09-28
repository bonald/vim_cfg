
#include "proto.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_kernel_if_define.h"
#include "gen/tbl_kernel_if.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_iproute_node_add_fail_define.h"
#include "gen/tbl_iproute_node_add_fail.h"
#include "gen/tbl_iproute_node_add_fail_count_define.h"
#include "gen/tbl_iproute_node_add_fail_count.h"
#include "gen/tbl_static_route_cfg_define.h"
#include "gen/tbl_static_route_cfg.h"
#include "gen/tbl_static_rt_cnt_define.h"
#include "gen/tbl_static_rt_cnt.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_ecmp_global_define.h"
#include "gen/tbl_ecmp_global.h"
#include "gen/tbl_rr_prefix_define.h"
#include "gen/tbl_rr_prefix.h"
#include "gen/tbl_nexthop_group_define.h"
#include "gen/tbl_nexthop_group.h"
#include "gen/tbl_bgp_define.h"
#include "gen/tbl_bgp.h"
#include "gen/tbl_bgp_network_define.h"
#include "gen/tbl_bgp_network.h"
#include "gen/tbl_bgp_neighbor_define.h"
#include "gen/tbl_bgp_neighbor.h"
#include "gen/tbl_rip_define.h"
#include "gen/tbl_rip.h"
#include "gen/tbl_rip_network_define.h"
#include "gen/tbl_rip_network.h"
#include "gen/tbl_rip_neighbor_define.h"
#include "gen/tbl_rip_neighbor.h"
#include "gen/tbl_rip_passive_if_define.h"
#include "gen/tbl_rip_passive_if.h"

#include "gen/tbl_ripng_define.h"
#include "gen/tbl_ripng.h"
#include "gen/tbl_ripng_network_define.h"
#include "gen/tbl_ripng_network.h"
#include "gen/tbl_ripng_passive_if_define.h"
#include "gen/tbl_ripng_passive_if.h"

#include "gen/tbl_ospf_define.h"
#include "gen/tbl_ospf.h"
#include "gen/tbl_ospf_network_define.h"
#include "gen/tbl_ospf_network.h"
#include "gen/tbl_ospf_area_auth_define.h"
#include "gen/tbl_ospf_area_auth.h"
#include "gen/tbl_ospf_area_range_define.h"
#include "gen/tbl_ospf_area_range.h"
#include "gen/tbl_ospf_area_stub_define.h"
#include "gen/tbl_ospf_area_stub.h"
#include "gen/tbl_ospf_area_nssa_define.h"
#include "gen/tbl_ospf_area_nssa.h"
#include "gen/tbl_ospf_area_vlink_define.h"
#include "gen/tbl_ospf_area_vlink.h"
#include "gen/tbl_sys_global_define.h"

#include "gen/tbl_ospf6_define.h"
#include "gen/tbl_ospf6.h"
#include "gen/tbl_ospf6_area_range_define.h"
#include "gen/tbl_ospf6_area_range.h"
#include "gen/tbl_ospf6_area_stub_define.h"
#include "gen/tbl_ospf6_area_stub.h"
#include "gen/tbl_ospf6_interface_define.h"
#include "gen/tbl_ospf6_interface.h"

#include "gen/tbl_sys_global.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_track_object_define.h"
#include "gen/tbl_track_object.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_sys_spec.h"
#include "gen/tbl_chassis.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "gen/tbl_nexthop_group_routed_define.h"
#include "gen/tbl_nexthop_group_routed.h"
#include "sal.h"
#include "sal_types.h"
#include "lib_fs.h"
#include "route.h"
#include "lib_netlink.h"
#include "rt_fea.h"
#include "rt_if.h"
#include "track.h"
#include "track_api.h"
#include "nd_api.h"
#include "nd.h"
#include "arp_define.h"
#include "arp_debug.h"
#include "arp_api.h"

#define RT_CMD_MULTIPATH_CHECK      8
#define RT_STATIC_MAX_CNT           1024
static uint32 bgp_key_id = 0;

void
cfg_parser_cmd_args_ip_masklen(char **argv, int32 argc, cfg_cmd_para_t *para, 
                              char* key_str, char* key_value_str, uint8* prefixlen, bool skip_key)
{
    int32 i = 0;
    
    for (skip_key ? (i = 1): (i = 0); i < argc; i++)
    {
        /* match key name */
        if (0 == sal_strcmp(argv[i], key_str))
        {
            i++;
            if (NULL != argv[i])
            {
                sal_strcpy(key_value_str, argv[i]);
            }

            /*get mask len*/
            i++;
            if (NULL != argv[i])
            {
                *prefixlen = sal_atoi(argv[i]);
            }
            break;
        }
    }

    return;
}

static int32
_rt_cmd_get_ecmp_global(char **argv, int32 argc, cfg_cmd_para_t *para, cdb_node_t  *p_node)
{
    FILE                   *fp = NULL;
    tbl_iter_args_t         args;
    tbl_ecmp_global_t      *p_db_ecmp_glb = NULL;
    int32                   field_id = 0;
    field_parse_t           field;

    sal_memset(&args, 0 ,sizeof(args));

    /* check exist */
    p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
    if (!p_db_ecmp_glb)
    {
        CFG_CONFLICT_RET("Tbl_ecmp_global not found");
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

    tbl_ecmp_global_dump_one(p_db_ecmp_glb, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_process_ecmp_hash_key(tbl_ecmp_global_t *p_db_ecmp_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32      rc = PM_E_NONE;
    char  **ppTypeArr  = NULL;
    uint32   hash_keys = 0;
    unsigned long nCnt = 0;
    uint32        i    = 0;
    
    if (split_tokens(argv[1], strlen(argv[1]), 16, ",", &nCnt, &ppTypeArr) != 0) 
    {
        return -1;
    }
    for (i = 0; i < nCnt; i++)
    {
        if (!sal_strcmp(ppTypeArr[i], "macsa"))
        {
            /*modified by chenc for bug 52381, 2019-06-16*/
            #ifdef GREATBELT
                CFG_FUNC_ERR_RET("Not support macsa into hash keys on %s product", lib_get_product_name());
            #endif
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_MACSA);
        }
        if (!sal_strcmp(ppTypeArr[i], "macda"))
        {
            /*modified by chenc for bug 52381, 2019-06-16*/
            #ifdef GREATBELT
                CFG_FUNC_ERR_RET("Not support macda into hash keys on %s product", lib_get_product_name());
            #endif
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_MACDA);
        }
        if (!sal_strcmp(ppTypeArr[i], "ethertype"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support ethertype into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_ETHERTYPE);
        }
        if (!sal_strcmp(ppTypeArr[i], "ipsa"))
        {
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_IPSA);
        }
        if (!sal_strcmp(ppTypeArr[i], "ipda"))
        {
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_IPDA);
        }
        if (!sal_strcmp(ppTypeArr[i], "ip-protocol"))
        {
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_IPPRO);
        }
        if (!sal_strcmp(ppTypeArr[i], "src-port"))
        {
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_SRC_PORT);
        }
        if (!sal_strcmp(ppTypeArr[i], "dst-port"))
        {
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_DST_PORT);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-macsa"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_MACSA);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-macda"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_MACDA);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-ipsa"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_IPSA);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-ipda"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_IPDA);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-ip-protocol"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-src-port"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_SRC_PORT);
        }
        if (!sal_strcmp(ppTypeArr[i], "inner-dst-port"))
        {
            if(!check_feature_support(CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH))
            {
                CFG_FUNC_ERR_RET("Not support inner field into hash keys on %s product", lib_get_product_name());
            }
            GLB_SET_FLAG(hash_keys, GLB_ECMP_HASH_KEY_INNER_DST_PORT);
        }
    }
    free_tokens(&ppTypeArr);

    p_db_ecmp_glb->hash_key = hash_keys;
    PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_HASH_KEY));

    return rc;
}

static int32
_rt_cmd_process_ecmp_hash_algorithm(tbl_ecmp_global_t *p_db_ecmp_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32   rc = PM_E_NONE;

    if (!sal_strcmp(argv[1], "xor") && p_db_ecmp_glb->hash_algorithm != GLB_ECMP_HASH_ALGO_XOR)
    {
        p_db_ecmp_glb->hash_algorithm = GLB_ECMP_HASH_ALGO_XOR;
    }
    else if (!sal_strcmp(argv[1], "crc") && p_db_ecmp_glb->hash_algorithm != GLB_ECMP_HASH_ALGO_CRC)
    {
        p_db_ecmp_glb->hash_algorithm = GLB_ECMP_HASH_ALGO_CRC;
    }
    else
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM));

    return rc;
}

uint32
rt_api_is_ipv6_profile()
{
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;    
    if (GLB_STM_MODE_IPUCV6 == type) 
    {
        return TRUE;
    }

    return FALSE;
}

uint32
rt_api_is_ipv6_profile_for_gg_gb()
{
#if defined (GOLDENGATE) || defined (GREATBELT)
     /*tbl_chassis_t   *p_chassis  = NULL;
     int32               type = 0;

    p_chassis = tbl_chassis_get_chassis(); 
    if (NULL == p_chassis) 
    { 
        return FALSE;
    }

    type = p_chassis->cur_stm_mode;

    if (GLB_STM_MODE_OF_IPV6 == type) 
    {
        return TRUE;
    }*/

    return TRUE;
#endif

    return FALSE;
}


static int32
_rt_is_ecmp_rrprefix_exist(addr_ipv4_t dest, uint8 prefixlen, cfg_cmd_para_t *para)
{
    uint32             rrprefix_index = 0;
    tbl_rr_prefix_t    *p_db_rrprefix = NULL;
    tbl_rr_prefix_master_t  *p_master = NULL;

    p_master = tbl_rr_prefix_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_rr_prefix not exist");
    }

    for (rrprefix_index = 0; rrprefix_index < GLB_MAX_ECMP_ROUND_ROBIN; rrprefix_index++)
    {
        p_db_rrprefix = p_master->rr_prefix_array[rrprefix_index];
        if (!p_db_rrprefix)
        {
            continue;
        }

        if (!sal_memcmp(&(p_db_rrprefix->rr_prefix.u.prefix4), &dest, sizeof(addr_ipv4_t)) 
          && prefixlen == p_db_rrprefix->rr_prefix.prefixlen)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static int32
_rt_cmd_ecmp_update_rr(tbl_rr_prefix_t *p_db_rrprefix, uint32 is_add)
{
    int32  rc  =  PM_E_NONE;
    tbl_ecmp_global_t       *p_db_ecmp_glb = NULL;
    tbl_iproute_node_t      *p_iproute_node = NULL;
    tbl_iproute_node_key_t   rn_key;
    tbl_nexthop_group_t     *p_nhg = NULL;
    tbl_nexthop_group_key_t  nhg_key;

    sal_memset(&rn_key, 0, sizeof(rn_key));
    sal_memset(&nhg_key, 0, sizeof(nhg_key));
    rn_key.vrf_id = 0;
    rn_key.p.family = AF_INET;
    rn_key.p.prefixlen = p_db_rrprefix->rr_prefix.prefixlen;
    sal_memcpy(&rn_key.p.u.prefix4, &(p_db_rrprefix->rr_prefix.u.prefix4), sizeof(addr_ipv4_t));

    p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
    if (!p_db_ecmp_glb)
    {
        return PM_E_NONE;
    }

    p_iproute_node = tbl_iproute_node_get_iproute_node(&rn_key);
    /* check exist */
    if (NULL == p_iproute_node)
    {
        return PM_E_NOT_EXIST;
    }
    /* check ecmp */
    if (!p_iproute_node->multipath)
    {
        return PM_E_NOT_EXIST;
    }

    if (p_iproute_node)
    {
        return PM_E_EXIST;
    }
    
    sal_memcpy(&nhg_key, &(p_iproute_node->nh_group), sizeof(nexthop_ecmp_t));
    p_nhg = tbl_nexthop_group_get_nexthop_group(&nhg_key);
    /* check exist */
    if (NULL == p_nhg)
    {
        return PM_E_NOT_EXIST;
    }

    if ((is_add && p_nhg->ecmp_hash == GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE)
        || (!is_add && p_nhg->ecmp_hash != GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE))
    {
        return PM_E_NONE;
    }

    if (is_add)
    {
        p_nhg->ecmp_hash = GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE;
    }
    else
    {
        p_nhg->ecmp_hash = p_db_ecmp_glb->load_balance_mode;
    }
    tbl_nexthop_group_set_nexthop_group_field(p_nhg, TBL_NEXTHOP_GROUP_FLD_ECMP_HASH);

    /* BUG 50205 update for ipda ecmpGroupId */
    tbl_iproute_node_set_iproute_node_field(p_iproute_node, TBL_IPROUTE_NODE_FLD_NH_GROUP);

    return rc;
}

/*added by chenc for bug 52749, 2019-07-01*/
static int32
_rt_cmd_ecmp_add_rrprefix_check_exist(tbl_rr_prefix_t *p_db_rrprefix)
{
    tbl_iproute_node_t      *p_iproute_node = NULL;
    tbl_iproute_node_key_t   rn_key;

    sal_memset(&rn_key, 0, sizeof(rn_key));
    rn_key.vrf_id = 0;
    rn_key.p.family = AF_INET;
    rn_key.p.prefixlen = p_db_rrprefix->rr_prefix.prefixlen;
    sal_memcpy(&rn_key.p.u.prefix4, &(p_db_rrprefix->rr_prefix.u.prefix4), sizeof(addr_ipv4_t));

    p_iproute_node = tbl_iproute_node_get_iproute_node(&rn_key);
    /* check exist */
    if (p_iproute_node)
    {
        return TRUE;
    }

    else
    {
        return FALSE;
    }
    
}
/*chenc end*/

static int32
_rt_cmd_ecmp_add_rrprefix(addr_ipv4_t dest, uint8 prefixlen, cfg_cmd_para_t *para)
{
    int32               rc  =  PM_E_NONE;
    uint8               rrprefix_index = 0;
    tbl_rr_prefix_t    *p_db_rrprefix = NULL;
    tbl_rr_prefix_t     rrprefix;
    
    tbl_rr_prefix_master_t  *p_master = NULL;

    p_master = tbl_rr_prefix_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_rr_prefix not exist");
    }

    for (rrprefix_index = 0; rrprefix_index < GLB_MAX_ECMP_ROUND_ROBIN; rrprefix_index++)
    {
        p_db_rrprefix = p_master->rr_prefix_array[rrprefix_index];
        if (NULL == p_db_rrprefix)
        {
            sal_memset(&rrprefix, 0, sizeof(tbl_rr_prefix_t));
            rrprefix.key.rr_prefix_index = rrprefix_index;
            rrprefix.rr_prefix.family = AF_INET;
            rrprefix.rr_prefix.prefixlen = prefixlen;
            sal_memcpy (&rrprefix.rr_prefix.u.prefix4, &dest, sizeof(addr_ipv4_t));

            /*added by chenc for bug 52749, 2019-07-01*/
            if (_rt_cmd_ecmp_add_rrprefix_check_exist(&rrprefix))
            {
                CFG_CONFLICT_RET("The prefix is already existed in ip route, please clear it first");
            }
            /*chenc end*/

            PM_E_RETURN(tbl_rr_prefix_add_rr_prefix(&rrprefix));

            p_db_rrprefix = tbl_rr_prefix_get_rr_prefix(&rrprefix.key);
            if (!p_db_rrprefix)
            {
                CFG_CONFLICT_RET("Ecmp round-robin prefix add failed");
            }
            rc = _rt_cmd_ecmp_update_rr(p_db_rrprefix, TRUE);
            if (rc == PM_E_EXIST)
            {
                CFG_CONFLICT_RET("The prefix is already existed in ip route, please clear it first");
            }
            else
            {
                /*not exist is not error*/
                rc = 0;
            }
            break;
        }
    }

    return rc;
}

/* Check if prefix is not 0/127/224. */
int
_rt_cmd_ipv4_prefix_check (addr_ipv4_t *addr)
{
    //struct sal_in4_addr addr;
    //struct sal_in4_addr mask;

    /* 32 bit address prefix is disallowed only on broadcast interfaces */
    //masklen2ip (p->prefixlen, &mask);
    //addr.s_addr = p->prefix.s_addr & mask.s_addr;

    if (IN_LOOPBACK(sal_ntoh32(addr->s_addr)) ||
        IN_EXPERIMENTAL(sal_ntoh32(addr->s_addr)))
    {
        return 0;
    }

    return 1;
}

int32
_rt_cmd_has_lag_interface()
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_if_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_if_lag, listnode)
    {
        if (INTERFACE_IS_LAG_GROUP(p_if_lag))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static int32
_rt_cmd_process_ecmp_load_balance(tbl_ecmp_global_t *p_db_ecmp_glb, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32        rc  =  PM_E_NONE;
    addr_ipv4_t  dest;
    uint8        prefixlen = 0;
    char         buf[64];
    char         ip_addr[GLB_IPV4_STR_LEN];
    uint32       mask = 0;
    
    sal_memset(&dest, 0, sizeof(addr_ipv4_t));

    if (!sal_strcmp(argv[1], "static"))
    {
        /*modified by chenc for bug 51385, 2019-06-16*/
        #ifdef GREATBELT
        CFG_INVALID_PARAM_RET("Not supported on this product");
        #endif
        
        if (p_db_ecmp_glb->load_balance_mode != GLB_ECMP_STATIC_LOAD_BALANCE)
        {
            p_db_ecmp_glb->load_balance_mode = GLB_ECMP_STATIC_LOAD_BALANCE;
        }

        if (p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL)
        {
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL;
        }
    }
    else if (!sal_strcmp(argv[1], "self-healing"))
    {
        /*modified by chenc for bug 51385, 2019-06-16*/
        #ifdef GREATBELT
        CFG_INVALID_PARAM_RET("Not supported on this product");
        #endif

        /*added by chenc for bug 52404 in 2019-06-03*/
        if (_rt_cmd_has_lag_interface())
        {
            CFG_INVALID_PARAM_RET("It can't support ecmp self-healing mode when linkagg has been configured");
        }
        /*chenc end*/
        
        if (p_db_ecmp_glb->load_balance_mode != GLB_ECMP_SELF_HEALING_LOAD_BALANCE)
        {
            p_db_ecmp_glb->load_balance_mode = GLB_ECMP_SELF_HEALING_LOAD_BALANCE;
        }

        if (p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL)
        {
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL;
        }
    }
    else if (!sal_strcmp(argv[1], "round-robin") && !sal_strcmp(argv[2], "rr_prefix"))
    {
        /*modified by chenc for bug 52377, 2019-06-14*/
        #ifdef GREATBELT
        CFG_INVALID_PARAM_RET("Not supported on this product");
        #endif
        
        cfg_parser_cmd_args_ip_masklen(argv, argc, para, "rr_prefix", ip_addr, &prefixlen, FALSE);
        if (0 != sal_strlen(ip_addr))
        {
            /*get ip*/
            rc = sal_inet_pton(AF_INET, ip_addr, buf);
            if (!rc)
            {
                CFG_INVALID_PARAM_RET("Invalid cli IP address");
            }

            sal_memcpy(&dest, buf, sizeof(addr_ipv4_t));
            IPV4_LEN_TO_MASK(mask, prefixlen);
            dest.s_addr = sal_htonl(dest.s_addr);
            dest.s_addr = dest.s_addr & mask;
            dest.s_addr = sal_htonl(dest.s_addr);
            if (!_rt_cmd_ipv4_prefix_check(&dest))
            {
                CFG_INVALID_PARAM_RET("Invalid IPv4 address");
            }
        }

        if (!_rt_is_ecmp_rrprefix_exist(dest, prefixlen, para))
        {
            if (p_db_ecmp_glb->rr_prefix_num >= GLB_MAX_ECMP_ROUND_ROBIN)
            {
                CFG_CONFLICT_RET("Only 16 round-robin prefix can be supported");
            }

            rc = _rt_cmd_ecmp_add_rrprefix(dest, prefixlen, para);
            if (PM_E_NONE == rc)
            {
                ++p_db_ecmp_glb->rr_prefix_num;
                PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM));
            }
        }

        return rc;
    }
    else if (!sal_strcmp(argv[1], "dynamic"))
    {
        /*added by chenc for bug 52404 in 2019-06-03*/
        if (_rt_cmd_has_lag_interface())
        {
            CFG_INVALID_PARAM_RET("It can't support ecmp dlb mode when linkagg has been configured");
        }
        /*chenc end*/

        if (p_db_ecmp_glb->load_balance_mode != GLB_ECMP_DYNAMIC_LOAD_BALANCE)
        {
            p_db_ecmp_glb->load_balance_mode = GLB_ECMP_DYNAMIC_LOAD_BALANCE;
        }

        if (!sal_strcmp(argv[3], "efd-only") && argc > 4)
        {
            if (p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT)
            {
                CFG_CONFLICT_RET("Current dynamic load balance is not efd-only");
            }
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL;
            PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE));
            return PM_E_NONE;
        }

        if (!sal_strcmp(argv[3], "tcp-only") && argc > 4)
        {
            if (p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP)
            {
                CFG_CONFLICT_RET("Current dynamic load balance is not tcp-only");
            }
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL;
            PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE));
            return PM_E_NONE;
        }

        if (!sal_strcmp(argv[3], "efd-only") && p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT)
        {
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT;
        }
        else if (!sal_strcmp(argv[3], "tcp-only") && p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP)
        {
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP;
        }
        else if (p_db_ecmp_glb->dlb_mode_type != GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL)
        {
            p_db_ecmp_glb->dlb_mode_type = GLB_ECMP_DYNAMIC_LOAD_BALANCE_NORMAL;
        }
    }
    else
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE));
    PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE));

    return rc;
}

int32
rt_cmd_process_ecmp_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                  rc = PM_E_NONE;
    cdb_node_t            *p_node = NULL;
    tbl_ecmp_global_t      ecmp_glb;
    tbl_ecmp_global_t     *p_db_ecmp_glb = NULL;
    int32                  field_id = 0;
    cdb_field_t           *p_field = NULL;
    uint32                 i = 0;

    CFG_DBG_FUNC();

    sal_memset(&ecmp_glb, 0, sizeof(tbl_ecmp_global_t));

    p_node = cdb_get_tbl(TBL_ECMP_GLOBAL);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
        if (!p_db_ecmp_glb)
        {
            CFG_CONFLICT_RET("Tbl_ecmp_global not found");
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
        case TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = _rt_cmd_process_ecmp_hash_algorithm(p_db_ecmp_glb, argv, argc, para);
            break;

        case TBL_ECMP_GLOBAL_FLD_LOAD_BALANCE_MODE:
            PM_CFG_CHECK_MIN_PARAMS(argc, 4, para->p_rs);
            rc = _rt_cmd_process_ecmp_load_balance(p_db_ecmp_glb, argv, argc, para);
            break;

        case TBL_ECMP_GLOBAL_FLD_HASH_KEY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            rc = _rt_cmd_process_ecmp_hash_key(p_db_ecmp_glb, argv, argc, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        break;

    case CDB_OPER_GET:
        rc = _rt_cmd_get_ecmp_global(argv, argc, para, p_node);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_rt_cmd_get_rr_prefix(uint32  rrprefix_index, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE                 *fp = NULL;
    tbl_rr_prefix_key_t   key;
    tbl_rr_prefix_t      *p_db_rrprefix = NULL;
    tbl_iter_args_t       args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (0 == rrprefix_index)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_rr_prefix_dump_one, &args);
    }
    else
    {
        key.rr_prefix_index = rrprefix_index;
        p_db_rrprefix = tbl_rr_prefix_get_rr_prefix(&key);
        if (!p_db_rrprefix)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("rr_prefix %u does not exist", rrprefix_index);
        }

        tbl_rr_prefix_dump_one(p_db_rrprefix, &args);
    }

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
rt_cmd_process_rr_prefix(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32              rc = PM_E_NONE;
    cdb_node_t        *p_node = NULL;
    field_parse_t      field;
    uint32             rrprefix_index = 0;
    addr_ipv4_t        dest;
    uint8              prefixlen = 0;
    char               buf[64];
    char               ip_addr[GLB_IPV4_STR_LEN];
    tbl_rr_prefix_t    *p_db_rrprefix = NULL;
    tbl_rr_prefix_master_t  *p_master = NULL;
    tbl_ecmp_global_t  *p_db_ecmp_glb = NULL;
    uint32       mask = 0;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_RR_PREFIX);
    sal_memset(&dest, 0, sizeof(addr_ipv4_t));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (1 == argc)
        {
            PM_CFG_GET_UINT(rrprefix_index, argv[0], para->p_rs);
        }

        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _rt_cmd_get_rr_prefix(rrprefix_index, para, p_node, &field);
        break;

    case CDB_OPER_DEL:
        /*modified by chenc for bug 52377, 2019-06-14*/
        #ifdef GREATBELT
        CFG_INVALID_PARAM_RET("Not supported on this product");
        #endif
        
        cfg_parser_cmd_args_ip_masklen(argv, argc, para, "rr_prefix", ip_addr, &prefixlen, FALSE);
        if (0 != sal_strlen(ip_addr))
        {
            /*get ip*/
            rc = sal_inet_pton(AF_INET, ip_addr, buf);
            if (!rc)
            {
                CFG_INVALID_PARAM_RET("Invalid cli IP address");
            }

            sal_memcpy(&dest, buf, sizeof(addr_ipv4_t));
            IPV4_LEN_TO_MASK(mask, prefixlen);
            dest.s_addr = sal_htonl(dest.s_addr);
            dest.s_addr = dest.s_addr & mask;
            dest.s_addr = sal_htonl(dest.s_addr);
            if (!_rt_cmd_ipv4_prefix_check(&dest))
            {
                CFG_INVALID_PARAM_RET("Invalid IPv4 address");
            }
        }

        p_master = tbl_rr_prefix_get_master();
        if (!p_master)
        {
            CFG_CONFLICT_RET("Tbl_rr_prefix not exist");
        }

        p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
        if (!p_db_ecmp_glb)
        {
            CFG_CONFLICT_RET("Tbl_ecmp_global not found");
        }

        for (rrprefix_index = 0; rrprefix_index < GLB_MAX_ECMP_ROUND_ROBIN; rrprefix_index++)
        {
            p_db_rrprefix = p_master->rr_prefix_array[rrprefix_index];
            if (!p_db_rrprefix)
            {
                continue;
            }

            if (!sal_memcmp(&(p_db_rrprefix->rr_prefix.u.prefix4), &dest, sizeof(addr_ipv4_t)) 
              && prefixlen ==  p_db_rrprefix->rr_prefix.prefixlen)
            {
                rc = _rt_cmd_ecmp_update_rr(p_db_rrprefix, FALSE);
                if (rc == PM_E_EXIST)
                {
                    CFG_CONFLICT_RET("The prefix is already existed in ip route, please clear it first");
                }

                rc = tbl_rr_prefix_del_rr_prefix(&(p_db_rrprefix->key));
                if (PM_E_NONE == rc)
                {
                    --p_db_ecmp_glb->rr_prefix_num;
                    PM_E_RETURN(tbl_ecmp_global_set_ecmp_global_field(p_db_ecmp_glb, TBL_ECMP_GLOBAL_FLD_RR_PREFIX_NUM));
                }
                break;
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
rt_cmd_cmd_show_ecmp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32                rc        = PM_E_NONE;
    FILE                *fp        = NULL;
    uint32               rrprefix_index  = 0;
    uint32               rr_head_printed = 0;
#ifndef TSINGMA
    uint32               out_hash_exist  = 0;
#endif
    char                 buf[CMD_BUF_256];
    tbl_ecmp_global_t   *p_db_ecmp_glb = NULL;
    tbl_rr_prefix_t     *p_db_rrprefix = NULL;
    tbl_rr_prefix_master_t  *p_master = NULL;

    p_db_ecmp_glb = tbl_ecmp_global_get_ecmp_global();
    if (!p_db_ecmp_glb)
    {
        CFG_CONFLICT_RET("Tbl_ecmp_global not found");
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    /*modified by chenc for bug 51385,52377, 2019-06-16*/
    #ifndef GREATBELT
    sal_fprintf(fp, "ECMP load balance enable mode:");

    if(GLB_ECMP_SELF_HEALING_LOAD_BALANCE == p_db_ecmp_glb->load_balance_mode)
    {
        sal_fprintf(fp, " Self-healing\n");    
    }
    else if(GLB_ECMP_ROUND_ROBIN_LOAD_BALANCE == p_db_ecmp_glb->load_balance_mode)
    {
        sal_fprintf(fp, " Round-robin\n");    
    }     
    else if(GLB_ECMP_DYNAMIC_LOAD_BALANCE == p_db_ecmp_glb->load_balance_mode)
    {
        sal_fprintf(fp, " Dynamic\n"); 
        if(GLB_ECMP_DYNAMIC_LOAD_BALANCE_TCP == p_db_ecmp_glb->dlb_mode_type)
        {
           sal_fprintf(fp, "ECMP dynamic load balance mode: TCP-only\n");    
        }
        else if(GLB_ECMP_DYNAMIC_LOAD_BALANCE_ELEPHANT == p_db_ecmp_glb->dlb_mode_type)
        {
           sal_fprintf(fp, "ECMP dynamic load balance mode: EFD-only\n");           
        }
    }
    else
    {
        sal_fprintf(fp, " Static\n");
    }
    #endif
    
    p_master = tbl_rr_prefix_get_master();
    if (!p_master)
    {
        CFG_CONFLICT_RET("Tbl_rr_prefix not exist");
    }
    for (rrprefix_index = 0; rrprefix_index < GLB_MAX_ECMP_ROUND_ROBIN; rrprefix_index++)
    {
        p_db_rrprefix = p_master->rr_prefix_array[rrprefix_index];
        if (!p_db_rrprefix)
        {
            continue;
        }

        if (0 == rr_head_printed)
        {
            sal_fprintf(fp, "ECMP round-robin prefix:\n");
            rr_head_printed = 1;
        }

        sal_fprintf(fp, "    %s/%d\n", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_rrprefix->rr_prefix.u.prefix4), p_db_rrprefix->rr_prefix.prefixlen);
    }
    
#ifndef TSINGMA
    sal_fprintf(fp, "ECMP hash-field-select:\n    ");

    if (0 == p_db_ecmp_glb->hash_key)
    {
        sal_fprintf(fp, "ipsa ipda ip-protocol src-port dst-port\n");

        sal_fclose(fp);
        fp = NULL;
        return rc;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_MACSA))
    {
        sal_fprintf(fp, "macsa ");
        out_hash_exist = 1;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_MACDA))
    {
        sal_fprintf(fp, "macda ");
        out_hash_exist = 1;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_ETHERTYPE))
    {
        sal_fprintf(fp, "ethertype ");
        out_hash_exist = 1;
    }
    
    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_IPSA))
    {
        sal_fprintf(fp, "ipsa ");
        out_hash_exist = 1;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_IPDA))
    {
        sal_fprintf(fp, "ipda ");
        out_hash_exist = 1;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_IPPRO))
    {
        sal_fprintf(fp, "ip-protocol ");
        out_hash_exist = 1;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_SRC_PORT))
    {
        sal_fprintf(fp, "src-port ");
        out_hash_exist = 1;
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_DST_PORT))
    {
        sal_fprintf(fp, "dst-port ");
        out_hash_exist = 1;
    }

    if (out_hash_exist)
    {
        sal_fprintf(fp, "\n    ");
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_MACSA))
    {
        sal_fprintf(fp, "inner-macsa ");
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_MACDA))
    {
        sal_fprintf(fp, "inner-macda ");
    }
   
    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_IPSA))
    {
        sal_fprintf(fp, "inner-ipsa ");
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_IPDA))
    {
        sal_fprintf(fp, "inner-ipda ");
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL))
    {
        sal_fprintf(fp, "inner-ip-protocol ");
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_SRC_PORT))
    {
        sal_fprintf(fp, "inner-src-port ");
    }

    if (GLB_FLAG_ISSET(p_db_ecmp_glb->hash_key, GLB_ECMP_HASH_KEY_INNER_DST_PORT))
    {
        sal_fprintf(fp, "inner-dst-port ");
    }
#endif
    sal_fprintf(fp, "\n");

    sal_fclose(fp);
    fp = NULL;
    return rc;
}


static int32
_rt_cmd_get_route(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE            *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;

    tbl_iproute_node_iterate((TBL_ITER_CB_FUNC)tbl_iproute_node_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_route_count(cfg_cmd_para_t *para, cdb_node_t *p_node, uint32 *count)
{
    FILE            *fp = NULL;
    fp = sal_fopen("/tmp/get_count", "w+");

    sal_fprintf(fp, "The route entry count is %d!\n", *count);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_static_rt_cnt(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE            *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_static_rt_cnt_iterate((TBL_ITER_CB_FUNC)tbl_static_rt_cnt_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_static_route_cfg(char *ifname, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE            *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = ifname;

    tbl_static_route_cfg_iterate((TBL_ITER_CB_FUNC)tbl_static_route_cfg_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ospf_iterate((TBL_ITER_CB_FUNC)tbl_ospf_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf6(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ospf6_iterate((TBL_ITER_CB_FUNC)tbl_ospf6_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}


static int32
_rt_cmd_get_ospf_network(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ospf_network_iterate((TBL_ITER_CB_FUNC)tbl_ospf_network_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf_area_range(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ospf_area_range_iterate((TBL_ITER_CB_FUNC)tbl_ospf_area_range_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf_area_auth(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_ospf_area_auth_iterate((TBL_ITER_CB_FUNC)tbl_ospf_area_auth_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf_area_stub(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ospf_area_stub_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf_area_nssa(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ospf_area_nssa_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_get_ospf_area_vlink(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_ospf_area_vlink_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_rt_cmd_clear_ospf_global(tbl_ospf_t *p_db_ospf)
{
    char        cmd_str[128];

    if (NULL == p_db_ospf)
    {
        return PM_E_NONE;
    }

    if (p_db_ospf->router_id.s_addr) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no router-id");
        rt_vty_router_ospf_cmd(cmd_str);
    }
    
    if (p_db_ospf->default_originate) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no default-information originate");
        rt_vty_router_ospf_cmd(cmd_str);
    }

    if (p_db_ospf->redistribute) {
        if (GLB_IS_BIT_SET(p_db_ospf->redistribute, RT_ROUTE_CONNECT)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "no redistribute connected");
            rt_vty_router_ospf_cmd(cmd_str);
        }
        if (GLB_IS_BIT_SET(p_db_ospf->redistribute, RT_ROUTE_STATIC)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "no redistribute static");
            rt_vty_router_ospf_cmd(cmd_str);
        }
		if (GLB_IS_BIT_SET(p_db_ospf->redistribute, RT_ROUTE_RIP)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ospf\" -c \"no redistribute rip\" -c \"end\"");
            system(cmd_str);
        }	
    }

    if (NULL != p_db_ospf) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no router ospf");
        rt_vty_config_cmd(cmd_str);
    }

    return PM_E_NONE;
}

int32
_rt_cmd_clear_ospf_network(tbl_ospf_network_t *p_ospf_network)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t prefix_cp;

    if (NULL == p_ospf_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_ospf_network->key.u.prefix4, sizeof(addr_ipv4_t));

    sal_sprintf(cmd_str, "no network %s/%d area %u",
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp), p_ospf_network->key.prefixlen,
         p_ospf_network->area_id);
    rt_vty_router_ospf_cmd(cmd_str);

    system(cmd_str);

    return PM_E_NONE;
}

int32
_rt_cmd_clear_ospf_area_auth(tbl_ospf_area_auth_t *p_ospf_area_auth)
{
    char        cmd_str[128];

    if (NULL == p_ospf_area_auth)
    {
        return PM_E_NONE;
    }

    sal_memset(cmd_str, 0, sizeof(cmd_str));

    if (OSPF_AREA_AUTH_TYPE_MD5 == p_ospf_area_auth->auth_type)
    {
        sal_sprintf(cmd_str, "no area %u authentication",
             p_ospf_area_auth->key.areaid);
        rt_vty_router_ospf_cmd(cmd_str);
    }

    return PM_E_NONE;
}

static int32
_rt_cmd_del_ospf_one(tbl_ospf_t *p_ospf, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_ospf_global(p_ospf);
    return tbl_ospf_del_ospf(&p_ospf->key);
}

static int32
_rt_cmd_del_ospf6_one(tbl_ospf6_t *p_ospf6, tbl_iter_args_t *pargs)
{
    return tbl_ospf6_del_ospf6(&p_ospf6->key);
}


static int32
_rt_cmd_del_ospf_network_one(tbl_ospf_network_t *p_ospf_network, tbl_iter_args_t *pargs)
{
    /* modified by liwh for bug 44571, 2017-07-05 */
    //_rt_cmd_clear_ospf_network(p_ospf_network);
    /* liwh end */
    return tbl_ospf_network_del_ospf_network(&p_ospf_network->key);
}

static int32
_rt_cmd_del_ospf_area_auth_one(tbl_ospf_area_auth_t *p_ospf_area_auth, tbl_iter_args_t *pargs)
{
    /* modified by liwh for bug 44571, 2017-07-05 */
    //_rt_cmd_clear_ospf_area_auth(p_ospf_area_auth);
    /* liwh end */
    return tbl_ospf_area_auth_del_ospf_area_auth(&p_ospf_area_auth->key);
}

static int32
_rt_cmd_del_ospf_area_range_one(tbl_ospf_area_range_t *p_ospf_area_range, tbl_iter_args_t *pargs)
{
    return tbl_ospf_area_range_del_ospf_area_range(&p_ospf_area_range->key);
}

static int32
_rt_cmd_del_ospf_area_stub_one(tbl_ospf_area_stub_t *p_ospf_area_stub, tbl_iter_args_t *pargs)
{
    return tbl_ospf_area_stub_del_ospf_area_stub(&p_ospf_area_stub->key);
}

static int32
_rt_cmd_del_ospf_area_nssa_one(tbl_ospf_area_nssa_t *p_ospf_area_nssa, tbl_iter_args_t *pargs)
{
    return tbl_ospf_area_nssa_del_ospf_area_nssa(&p_ospf_area_nssa->key);
}

static int32
_rt_cmd_del_ospf_area_vlink_one(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_iter_args_t *pargs)
{
    return tbl_ospf_area_vlink_del_ospf_area_vlink(&p_ospf_area_vlink->key);
}

static int32
_rt_cmd_del_ospf_process(void)
{
    tbl_iter_args_t args;

    /*  1. delete the router ospf 
        2. delete the ospf  network
        3. delete the ospf area auth
        4. delete the ospf area range
    */

    tbl_ospf_network_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_network_one, &args);
    tbl_ospf_area_auth_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_area_auth_one, &args);
    tbl_ospf_area_range_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_area_range_one, &args);
	tbl_ospf_area_stub_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_area_stub_one, &args);
    tbl_ospf_area_nssa_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_area_nssa_one, &args);
    tbl_ospf_area_vlink_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_area_vlink_one, &args);
    tbl_ospf_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf_one, &args);

    return PM_E_NONE;
}

static int32
_rt_cmd_del_ospf6_interface_one(tbl_ospf6_interface_t *p_ospf6_interface, tbl_iter_args_t *pargs)
{
    return tbl_ospf6_interface_del_ospf6_interface(&p_ospf6_interface->key);
}

static int32
_rt_cmd_del_ospf6_area_range_one(tbl_ospf6_area_range_t *p_ospf6_area_range, tbl_iter_args_t *pargs)
{
    return tbl_ospf6_area_range_del_ospf6_area_range(&p_ospf6_area_range->key);
}

static int32
_rt_cmd_del_ospf6_area_stub_one(tbl_ospf6_area_stub_t *p_ospf6_area_stub, tbl_iter_args_t *pargs)
{
    return tbl_ospf6_area_stub_del_ospf6_area_stub(&p_ospf6_area_stub->key);
}


static int32
_rt_cmd_del_ospf6_process(void)
{
    tbl_iter_args_t args;

    /*  1. delete the router ospf6 
        2. delete the ospf6  interface
        3. delete the ospf6 area stub
        4. delete the ospf6 area range
    */

    tbl_ospf6_interface_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf6_interface_one, &args);
    tbl_ospf6_area_range_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf6_area_range_one, &args);
	tbl_ospf6_area_stub_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf6_area_stub_one, &args);
    tbl_ospf6_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ospf6_one, &args);

    return PM_E_NONE;
}


uint32
_rt_cmd_ospf_area_range_count_get_max()
{
    tbl_sys_spec_t  *p_sys_spec = NULL;
    tbl_chassis_t   *p_chassis  = NULL;
    int32 type = 1000;

    p_chassis = tbl_chassis_get_chassis();
    if (NULL == p_chassis) 
    {
        return 0;
    }

    type = p_chassis->cur_stm_mode;
    if (type < GLB_STM_MODE_MAX) 
    {
        p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
        if(NULL != p_sys_spec) 
        {
            return p_sys_spec->ospf_area_range_num;
        }
    }

    return 0;
}

uint32
_rt_cmd_ospf_area_range_count_max_check()
{
    uint32 is_over = FALSE;

    tbl_ospf_area_range_master_t   * master     = tbl_ospf_area_range_get_master();

    if (master && master->ospf_area_range_hash->count >= _rt_cmd_ospf_area_range_count_get_max())
    {
        is_over = TRUE;
    }

    return is_over;
}

static int32
_rt_cmd_clear_ospf_default_info_originate(tbl_ospf_t *p_db_ospf)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

    if (!p_db_ospf->default_originate)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "no default-information originate");
    rt_vty_router_ospf_cmd(cmd_str);

    return PM_E_NONE;
}

/* modified by liwh for bug 44589, 2017-07-10 */
static int32
_rt_cmd_set_ospf_router_id(tbl_ospf_t *p_db_ospf)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

	if (NULL == p_db_ospf || 0 == p_db_ospf->router_id.s_addr)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "router-id %s", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_ospf->router_id));
    rt_vty_router_ospf_cmd(cmd_str);

    return PM_E_NONE;
}
/* liwh end */

static int32
_rt_cmd_clear_bgp_global(tbl_bgp_t *p_db_bgp)
{
    char        cmd_str[128];
    char        buf[32];
    if (NULL == p_db_bgp)
    {
        return PM_E_NONE;
    }

    if (p_db_bgp->router_id.s_addr != 0) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no bgp router %s\" -c \"end\"",
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_bgp->router_id));
        system(cmd_str);
    }

    if (p_db_bgp->redistribute) {
        if (GLB_IS_BIT_SET(p_db_bgp->redistribute, RT_ROUTE_CONNECT)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no redistribute connected\" -c \"end\"");
            system(cmd_str);
        }
        if (GLB_IS_BIT_SET(p_db_bgp->redistribute, RT_ROUTE_STATIC)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no redistribute static\" -c \"end\"");
            system(cmd_str);
        }
		/*if (GLB_IS_BIT_SET(p_db_bgp->redistribute, RT_ROUTE_KERNEL)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no redistribute kernel\" -c \"end\"");
            system(cmd_str);
        }*/
        if (GLB_IS_BIT_SET(p_db_bgp->redistribute, RT_ROUTE_OSPF)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no redistribute ospf\" -c \"end\"");
            system(cmd_str);
        }
		if (GLB_IS_BIT_SET(p_db_bgp->redistribute, RT_ROUTE_RIP)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no redistribute rip\" -c \"end\"");
            system(cmd_str);
		}
    }
	
    if (NULL != p_db_bgp) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"no router bgp %s\" -c \"end\"",
            cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_bgp->router_id));
        system(cmd_str);
    }

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_bgp_network(tbl_bgp_network_t *p_bgp_network)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t prefix_cp;
    char        buff[CMD_BUF_256];    

    if (NULL == p_bgp_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buff, 0, sizeof(buff));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_bgp_network->key.u.prefix4, sizeof(addr_ipv4_t));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router bgp\" -c \"no network %s mask %s\" -c \"end\"",
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp), cdb_addr_ipv4_val2str(buff, CMD_BUF_256, &p_bgp_network->mask));

    system(cmd_str);

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_bgp_neighbor(tbl_bgp_neighbor_t *p_bgp_neighbor)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t prefix_cp;

    if (NULL == p_bgp_neighbor)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_bgp_neighbor->key.u.prefix4, sizeof(addr_ipv4_t));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no neighbor %s\" -c \"end\"",
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp));

    system(cmd_str);

    return PM_E_NONE;
}

static int32
_rt_cmd_del_bgp_one(tbl_bgp_t *p_bgp, tbl_iter_args_t *pargs)
{
    //_rt_cmd_clear_bgp_global(p_bgp);
    return tbl_bgp_del_bgp(&p_bgp->key);
}

static int32
_rt_cmd_del_bgp_network_one(tbl_bgp_network_t *p_bgp_network, tbl_iter_args_t *pargs)
{
    //_rt_cmd_clear_bgp_network(p_bgp_network);
    return tbl_bgp_network_del_bgp_network(&p_bgp_network->key);
}

static int32
_rt_cmd_del_bgp_neighbor_one(tbl_bgp_neighbor_t *p_bgp_neighbor, tbl_iter_args_t *pargs)
{
    //_rt_cmd_clear_bgp_neighbor(p_bgp_neighbor);
    return tbl_bgp_neighbor_del_bgp_neighbor(&p_bgp_neighbor->key);
}

static int32
_rt_cmd_del_bgp_process(void)
{
    tbl_iter_args_t args;

    /*  1. delete the router bgp 
        2. delete the bgp  network
        3. delete the bgp  neighbor
    */

    tbl_bgp_network_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_bgp_network_one, &args);
    tbl_bgp_neighbor_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_bgp_neighbor_one, &args);
    tbl_bgp_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_bgp_one, &args);

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_rip_global(tbl_rip_t *p_db_rip)
{
    char        cmd_str[128];

    if (NULL == p_db_rip)
    {
        return PM_E_NONE;
    }

    if (p_db_rip->version != 0) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no version\" -c \"end\"");
        system(cmd_str);
    }
    
    if (p_db_rip->allow_ecmp) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no allow_ecmp\" -c \"end\"");
        system(cmd_str);
    }

	if ((p_db_rip->timers.update!=0 || p_db_rip->timers.timeout!=0 || p_db_rip->timers.garbage!=0) 
		&& (p_db_rip->timers.update!=30 || p_db_rip->timers.timeout!=180 || p_db_rip->timers.garbage!=120)) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no timers basic\" -c \"end\"");
        system(cmd_str);
    }

	if (RIP_DEFAULT_INFORMATION == p_db_rip->default_information) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no default_information originate\" -c \"end\"");
        system(cmd_str);
    }

    if (p_db_rip->redistribute) {
        if (GLB_IS_BIT_SET(p_db_rip->redistribute, RT_ROUTE_CONNECT)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no redistribute connected\" -c \"end\"");
            system(cmd_str);
        }
        if (GLB_IS_BIT_SET(p_db_rip->redistribute, RT_ROUTE_STATIC)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no redistribute static\" -c \"end\"");
            system(cmd_str);
        }
		if (GLB_IS_BIT_SET(p_db_rip->redistribute, RT_ROUTE_KERNEL)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no redistribute kernel\" -c \"end\"");
            system(cmd_str);
        }
        if (GLB_IS_BIT_SET(p_db_rip->redistribute, RT_ROUTE_OSPF)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no redistribute ospf\" -c \"end\"");
            system(cmd_str);
        }
		if (GLB_IS_BIT_SET(p_db_rip->redistribute, RT_ROUTE_BGP)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no redistribute bgp\" -c \"end\"");
            system(cmd_str);
		}
    }

	if (p_db_rip->default_metric > 1) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no default_metric\" -c \"end\"");
        system(cmd_str);
    }

	if (p_db_rip->passive_if_default) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no passive-interface default\" -c \"end\"");
        system(cmd_str);
    }
	
    if (NULL != p_db_rip) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"no router rip\" -c \"end\"");
        system(cmd_str);
    }

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_ripng_global(tbl_ripng_t *p_db_ripng)
{
    char        cmd_str[128];

    if (NULL == p_db_ripng)
    {
        return PM_E_NONE;
    }

    if (p_db_ripng->allow_ecmp) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no allow_ecmp\" -c \"end\"");
        system(cmd_str);
    }

	if ((p_db_ripng->timers.update!=0 || p_db_ripng->timers.timeout!=0 || p_db_ripng->timers.garbage!=0) 
		&& (p_db_ripng->timers.update!=30 || p_db_ripng->timers.timeout!=180 || p_db_ripng->timers.garbage!=120)) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no timers basic\" -c \"end\"");
        system(cmd_str);
    }

	if (RIPNG_DEFAULT_INFORMATION == p_db_ripng->default_information) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no default_information originate\" -c \"end\"");
        system(cmd_str);
    }

    if (p_db_ripng->redistribute) {
        if (GLB_IS_BIT_SET(p_db_ripng->redistribute, RT_ROUTE_CONNECT)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no redistribute connected\" -c \"end\"");
            system(cmd_str);
        }
        if (GLB_IS_BIT_SET(p_db_ripng->redistribute, RT_ROUTE_STATIC)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no redistribute static\" -c \"end\"");
            system(cmd_str);
        }
		if (GLB_IS_BIT_SET(p_db_ripng->redistribute, RT_ROUTE_KERNEL)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no redistribute kernel\" -c \"end\"");
            system(cmd_str);
        }
        if (GLB_IS_BIT_SET(p_db_ripng->redistribute, RT_ROUTE_OSPF)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no redistribute ospf\" -c \"end\"");
            system(cmd_str);
        }
		if (GLB_IS_BIT_SET(p_db_ripng->redistribute, RT_ROUTE_BGP)) {
            sal_memset(cmd_str, 0, sizeof(cmd_str));
            sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no redistribute bgp\" -c \"end\"");
            system(cmd_str);
		}
    }

	if (p_db_ripng->default_metric > 1) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no default_metric\" -c \"end\"");
        system(cmd_str);
    }

    if (NULL != p_db_ripng) {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"no router ripng\" -c \"end\"");
        system(cmd_str);
    }

    return PM_E_NONE;
}


static int32
_rt_cmd_clear_rip_network(tbl_rip_network_t *p_rip_network)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t prefix_cp;
    uint32      mask = 0;

    if (NULL == p_rip_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    IPV4_LEN_TO_MASK(mask, p_rip_network->key.prefixlen);
    sal_memcpy(&prefix_cp, &p_rip_network->key.u.prefix4, sizeof(addr_ipv4_t));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no network %s/%d\" -c \"end\"",
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp), p_rip_network->key.prefixlen);

    system(cmd_str);

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_ripng_network(tbl_ripng_network_t *p_ripng_network)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv6_t prefix_cp;
    //uint32      mask = 0;

    if (NULL == p_ripng_network)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    //IPV6_LEN_TO_MASK(mask, p_ripng_network->key.prefixlen);
    sal_memcpy(&prefix_cp, &p_ripng_network->key.u.prefix6, sizeof(addr_ipv6_t));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no network %s/%d\" -c \"end\"",
        cdb_addr_ipv6_val2str(buf, CMD_BUF_256, &prefix_cp), p_ripng_network->key.prefixlen);

    system(cmd_str);

    return PM_E_NONE;
}


static int32
_rt_cmd_clear_rip_neighbor(tbl_rip_neighbor_t *p_rip_neighbor)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    addr_ipv4_t prefix_cp;

    if (NULL == p_rip_neighbor)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(&prefix_cp, 0, sizeof(addr_ipv4_t));
    sal_memcpy(&prefix_cp, &p_rip_neighbor->key.u.prefix4, sizeof(addr_ipv4_t));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no neighbor %s\" -c \"end\"",
        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &prefix_cp));

    system(cmd_str);

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_rip_passive_if(tbl_rip_passive_if_t *p_rip_passive_if)
{
	char    cmd_str[128];
    char 	name[IFNAME_SIZE];

    if (NULL == p_rip_passive_if)
    {
        return PM_E_NONE;
    }

	sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(name, 0, sizeof(name));
    sal_memcpy(name, p_rip_passive_if->key.name, sizeof(name));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no passive-interface %s\" -c \"end\"",
        name);

    system(cmd_str);
    return PM_E_NONE;
}

static int32
_rt_cmd_clear_ripng_passive_if(tbl_ripng_passive_if_t *p_ripng_passive_if)
{
	char    cmd_str[128];
    char 	name[IFNAME_SIZE];

    if (NULL == p_ripng_passive_if)
    {
        return PM_E_NONE;
    }

	sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_memset(name, 0, sizeof(name));
    sal_memcpy(name, p_ripng_passive_if->key.name, sizeof(name));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no passive-interface %s\" -c \"end\"",
        name);

    system(cmd_str);
    return PM_E_NONE;
}


static int32
_rt_cmd_del_rip_one(tbl_rip_t *p_rip, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_rip_global(p_rip);
    return tbl_rip_del_rip(&p_rip->key);
}

static int32
_rt_cmd_del_ripng_one(tbl_ripng_t *p_ripng, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_ripng_global(p_ripng);
    return tbl_ripng_del_ripng(&p_ripng->key);
}


static int32
_rt_cmd_del_rip_network_one(tbl_rip_network_t *p_rip_network, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_rip_network(p_rip_network);
    return tbl_rip_network_del_rip_network(&p_rip_network->key);
}

static int32
_rt_cmd_del_ripng_network_one(tbl_ripng_network_t *p_ripng_network, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_ripng_network(p_ripng_network);
    return tbl_ripng_network_del_ripng_network(&p_ripng_network->key);
}


static int32
_rt_cmd_del_rip_neighbor_one(tbl_rip_neighbor_t *p_rip_neighbor, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_rip_neighbor(p_rip_neighbor);
    return tbl_rip_neighbor_del_rip_neighbor(&p_rip_neighbor->key);
}

static int32
_rt_cmd_del_rip_passive_if_one(tbl_rip_passive_if_t *p_rip_passive_if, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_rip_passive_if(p_rip_passive_if);
    return tbl_rip_passive_if_del_rip_passive_if(&p_rip_passive_if->key);
}

static int32
_rt_cmd_del_ripng_passive_if_one(tbl_ripng_passive_if_t *p_ripng_passive_if, tbl_iter_args_t *pargs)
{
    _rt_cmd_clear_ripng_passive_if(p_ripng_passive_if);
    return tbl_ripng_passive_if_del_ripng_passive_if(&p_ripng_passive_if->key);
}


static int32
_rt_cmd_del_rip_process(void)
{
    tbl_iter_args_t args;

    /*  1. delete the router rip 
        2. delete the rip  network
        3. delete the rip  neighbor
        4. delete the rip  passive-interface
    */

    tbl_rip_network_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_rip_network_one, &args);
    tbl_rip_neighbor_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_rip_neighbor_one, &args);
	tbl_rip_passive_if_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_rip_passive_if_one, &args);
    tbl_rip_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_rip_one, &args);

    return PM_E_NONE;
}

static int32
_rt_cmd_del_ripng_process(void)
{
    tbl_iter_args_t args;

    /*  1. delete the router ripng 
        2. delete the ripng  network
        3. delete the ripng  neighbor
        4. delete the ripng  passive-interface
    */

    tbl_ripng_network_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ripng_network_one, &args);
	tbl_ripng_passive_if_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ripng_passive_if_one, &args);
    tbl_ripng_iterate((TBL_ITER_CB_FUNC)_rt_cmd_del_ripng_one, &args);

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_rip_default_information(tbl_rip_t *p_db_rip)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

    if (!p_db_rip->default_information)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router rip\" -c \"no default-information originate\" -c \"end\"");

    system(cmd_str);

    return PM_E_NONE;
}

static int32
_rt_cmd_clear_ripng_default_information(tbl_ripng_t *p_db_ripng)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

    if (!p_db_ripng->default_information)
    {
        return PM_E_NONE;
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "vtysh -c \"configure terminal\" -c \"router ripng\" -c \"no default-information originate\" -c \"end\"");

    system(cmd_str);

    return PM_E_NONE;
}


int32
neighbor_addr_match(addr_t *n, addr_t *p)
{
	int32 offset = 0;

	/* Set both prefix's head pointer. */
	uint32 *np = (uint32*)&n->u.prefix4;
	uint32 *pp = (uint32*)&p->u.prefix4;

	for (;offset<32;offset++)
	{
		if (np[offset] != pp[offset])
		{
			return FALSE;
		}
	}

	return TRUE;
}

static int32
_rt_cmd_proto_redistnum(int32 afi, const char *s)
{
    if (!s) {
        return -1;
    }
    if (afi == AF_INET) {
		if (sal_strncmp (s, "ripng", 5) == 0)
            return RT_ROUTE_RIPNG;
        else if (sal_strncmp (s, "k", 1) == 0)
            return RT_ROUTE_KERNEL;
        else if (sal_strncmp (s, "c", 1) == 0)
            return RT_ROUTE_CONNECT;
        else if (sal_strncmp (s, "s", 1) == 0)
            return RT_ROUTE_STATIC;
        else if (sal_strncmp (s, "r", 1) == 0)
            return RT_ROUTE_RIP;
        else if (sal_strncmp (s, "o", 1) == 0)
            return RT_ROUTE_OSPF;
        else if (sal_strncmp (s, "i", 1) == 0)
            return RT_ROUTE_ISIS;
        else if (sal_strncmp (s, "bg", 2) == 0)
            return RT_ROUTE_BGP;
    }
    return -1;
}

uint32
rt_cli_get_ipv4_prefix(char *addr)
{
    uint32          addr_ipv4 = 0;;
    int32           retv;

    retv = sal_inet_pton(AF_INET, addr, (void*)&addr_ipv4);
    if (!retv)
    {
        sal_printf("Invalid cli IP address\n");
        return 0;
    }

    addr_ipv4 = sal_htonl(addr_ipv4);
    return addr_ipv4;
}

int32
route_ipv6_static_route_del_by_dest(void *obj, tbl_iter_args_t *pargs)
{
    tbl_static_route_cfg_t  *pst_static_route_cfg   = obj;
    addr_ipv6_t     *addr            = pargs->argv[0];
    int                 rc = PM_E_NONE;

    if (NULL == pst_static_route_cfg || NULL == addr)
    {
        return PM_E_FAIL;
    }

    if (AF_INET6 != pst_static_route_cfg->key.p.family)
    {
        return PM_E_NONE;
    }

    /*static route dest is same as if addr connect 128 bit route*/
    if (pst_static_route_cfg->key.p.prefixlen == IPV6_MAX_BITLEN
        && 0 == sal_memcmp(&pst_static_route_cfg->key.p.u.prefix6, addr, sizeof(pst_static_route_cfg->key.p.u.prefix6)))
    {
        rc = rt_if_del_static_route_v6_by_syscmd(&pst_static_route_cfg->key.p.u.prefix6, &pst_static_route_cfg->key.nh_addr.ipv6, pst_static_route_cfg->key.p.prefixlen, pst_static_route_cfg->key.ifindex_v6);
        //usleep(1000);
        
        rc = rt_if_add_static_route_v6_by_syscmd(&pst_static_route_cfg->key.p.u.prefix6, &pst_static_route_cfg->key.nh_addr.ipv6, pst_static_route_cfg->key.p.prefixlen, pst_static_route_cfg->distance, pst_static_route_cfg->key.ifindex_v6);
    }
    
    return rc;
}


/*if addr is 2.2.2.2/24, connect route 2.2.2.2/32, then config static route 2.2.2.2/32, delete if addr,here update static route*/
void
route_update_route_v6_when_del_if_addr(addr_ipv6_t *addr)
{
    tbl_iter_args_t args;

    args.argv[0] = addr;
    
    tbl_static_route_cfg_iterate(route_ipv6_static_route_del_by_dest, &args);
}

extern int32
_rt_if_cmd_del_linux_link_local(tbl_route_if_t *p_rt_if);

int32
rt_cmd_add_if_addr_v6_by_syscmd(char *name, addr_ipv6_t *addr, uint32 mask_len)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

    if (IN6_IS_ADDR_LINKLOCAL(addr))
    {
        //return PM_E_NONE;
        
        /*default linux generate a link-local address, then first configure link-local address, linux will exist 2 link-local, 
        so delete the linux link-local address first*/
        //if (GLB_FLAG_ISSET(p_conn->flags, RT_IF_ADDRV6_LINKLOCAL) && (CTCLIB_SLISTCOUNT(p_db_rtif->ifc_ipv6.obj_list) == 0))
        tbl_route_if_t *p_db_rtif = NULL;
            
        p_db_rtif = tbl_route_if_get_route_if_by_name(name);
        if (NULL == p_db_rtif)
        {
            return PM_E_FAIL;
        }
            
        if (0 == p_db_rtif->ipv6_link_local_had_delete)
        {
            _rt_if_cmd_del_linux_link_local(p_db_rtif);
            p_db_rtif->ipv6_link_local_had_delete = 1;
        }
    }
    
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "ipv6 address %s/%d", cdb_addr_ipv6_val2str(buf, CMD_BUF_256, addr), mask_len);
    rt_vty_if_cmd(name, cmd_str);

    route_update_route_v6_when_del_if_addr(addr);

    return PM_E_NONE;
}

int32
rt_cmd_del_if_addr_v6_by_syscmd(char *name, addr_ipv6_t *addr, uint32 mask_len)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

    if (IN6_IS_ADDR_LINKLOCAL(addr))
    {
        //return PM_E_NONE;
    }
    
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "no ipv6 address %s/%d", cdb_addr_ipv6_val2str(buf, CMD_BUF_256, addr), mask_len);
    rt_vty_if_cmd(name, cmd_str);
    usleep(10000);

    route_update_route_v6_when_del_if_addr(addr);

    return PM_E_NONE;
}

int32
route_ipv4_static_route_del_by_dest(void *obj, tbl_iter_args_t *pargs)
{
    tbl_static_route_cfg_t  *pst_static_route_cfg   = obj;
    addr_ipv4_t     *addr            = pargs->argv[0];
    int                 rc = PM_E_NONE;

    if (NULL == pst_static_route_cfg || NULL == addr)
    {
        return PM_E_FAIL;
    }

    if (AF_INET6 == pst_static_route_cfg->key.p.family)
    {
        return PM_E_NONE;
    }

    /*static route dest is same as if addr connect 32 bit route*/
    if (pst_static_route_cfg->key.p.prefixlen == IPV4_MAX_BITLEN
        && 0 == sal_memcmp(&pst_static_route_cfg->key.p.u.prefix4, addr, sizeof(pst_static_route_cfg->key.p.u.prefix4)))
    {
        rc = rt_if_del_static_route_by_syscmd(&pst_static_route_cfg->key.p.u.prefix4, &pst_static_route_cfg->key.nh_addr.ipv4, pst_static_route_cfg->key.p.prefixlen);
        //usleep(1000);
        
        rc = rt_if_add_static_route_by_syscmd(&pst_static_route_cfg->key.p.u.prefix4, &pst_static_route_cfg->key.nh_addr.ipv4, pst_static_route_cfg->key.p.prefixlen, pst_static_route_cfg->distance);
    }
    
    return rc;
}


/*if addr is 2.2.2.2/24, connect route 2.2.2.2/32, then config static route 2.2.2.2/32, delete if addr,here update static route*/
void
route_update_route_when_del_if_addr(addr_ipv4_t *addr)
{
    tbl_iter_args_t args;

    args.argv[0] = addr;
    
    tbl_static_route_cfg_iterate(route_ipv4_static_route_del_by_dest, &args);
}

int32
rt_cmd_add_if_addr_by_syscmd(char *name, addr_ipv4_t *addr, uint32 mask_len, uint32 update)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];
    tbl_interface_t *p_db_if = NULL;

    p_db_if = tbl_interface_get_interface_by_name(name);
    if (!p_db_if)
    {
        return PM_E_NONE;
    }
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "ip address %s/%d", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, addr), mask_len);
    rt_vty_if_cmd(name, cmd_str);

    prefix_t p;
    sal_memset(&p, 0x00, sizeof(p));

    p.family = AF_INET;
    p.prefixlen = mask_len;
    sal_memcpy(&p.u.prefix4, addr, sizeof(addr_ipv4_t));

    tbl_route_if_t *p_db_rtif = NULL;
    p_db_rtif = tbl_route_if_get_route_if_by_name(name);
    if (p_db_rtif)
    {
        if (tbl_interface_is_running(p_db_if))
        {
            MY_PRINTF("add local %x/%d", addr->s_addr, mask_len);
            kernel_add_local(&p, p_db_rtif->kernel_ifindex);
        }
        else
        {
            MY_PRINTF("del local %x/%d", addr->s_addr, mask_len);
            kernel_del_local(&p);
        }
    }

    if (update)
    {
        route_update_route_when_del_if_addr(addr);
    }

    return PM_E_NONE;
}

int32
rt_cmd_del_if_addr_by_syscmd(char *name, addr_ipv4_t *addr, uint32 mask_len, uint32 update)
{
    char        buf[CMD_BUF_256];
    char        cmd_str[128];

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    sal_sprintf(cmd_str, "no ip address %s/%d", cdb_addr_ipv4_val2str(buf, CMD_BUF_256, addr), mask_len);
    rt_vty_if_cmd(name, cmd_str);
    usleep(100000);

    if (update)
    {
        route_update_route_when_del_if_addr(addr);
    }

    return PM_E_NONE;
}

int32
if_cmd_process_check_l3if_exist(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_route_if_key_t key;
    tbl_route_if_t *p_db_rtif = NULL;

    sal_memset(&key, 0, sizeof(key));

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        sal_strcpy(key.name, argv[0]);
        p_db_rtif = tbl_route_if_get_route_if(&key);
        if (NULL == p_db_rtif)
        {
            CFG_INVALID_PARAM_RET("Route interface not exist");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return PM_E_NONE;
}

int32
if_cmd_process_check_l3if_addr(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    char addr_buf[CMD_BUF_32];
    addr_t addr;
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr, 0, sizeof(addr));
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        CFG_PARSER_IP_ADDR(argv[0], &addr.u.prefix4, para->p_rs);
        if (p_sys_glb->startup_done)
        {
            addr.family = AF_INET;
            if (FALSE == route_if_addr_exists(&addr))
            {
                CFG_CONFLICT_RET("IP address %s not configured", cdb_addr_val2str(addr_buf, CMD_BUF_32, &addr));
            }
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return PM_E_NONE;
}

int32
rt_cmd_addr_check_overlap(addr_t *p_addr, tbl_route_if_t **pp_overlap_rtif)
{
    prefix_t prefix;
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    tbl_route_if_t      *p_db_rtif_node = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *listnode_ds = NULL;
    ds_connected_t      *p_db_conn = NULL;
    ds_connected_v6_t   *p_db_conn_v6 = NULL;
    int32                rc = PM_E_NONE;

    *pp_overlap_rtif = NULL;

    sal_memset(&prefix, 0, sizeof(prefix));
    prefix.family = p_addr->family;
    prefix.prefixlen = p_addr->family == AF_INET ? IPV4_MAX_PREFIXLEN : IPV6_MAX_PREFIXLEN;
    prefix.u.prefix4 = p_addr->u.prefix4;
    prefix.u.prefix6 = p_addr->u.prefix6;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif_node, listnode)
    {
        if (AF_INET == p_addr->family)
        {
            if (CTCLIB_SLIST_ISEMPTY(p_db_rtif_node->ifc_ipv4.obj_list))
            {
                continue;
            }

            CTCLIB_SLIST_LOOP(p_db_rtif_node->ifc_ipv4.obj_list, p_db_conn, listnode_ds)
            {
                rc = prefix_compare_net(&p_db_conn->key.address, &prefix);
                if (!rc)
                {
                    continue;
                }

                *pp_overlap_rtif = p_db_rtif_node;
                return TRUE;
            }
        }
        else if (AF_INET6 == p_addr->family)
        {
            if (CTCLIB_SLIST_ISEMPTY(p_db_rtif_node->ifc_ipv6.obj_list))
            {
                continue;
            }

            CTCLIB_SLIST_LOOP(p_db_rtif_node->ifc_ipv6.obj_list, p_db_conn_v6, listnode_ds)
            {
                rc = prefix6_compare_net(&p_db_conn_v6->key.address, &prefix);
                if (!rc)
                {
                    continue;
                }

                *pp_overlap_rtif = p_db_rtif_node;
                return TRUE;
            }
        }
    }

    return FALSE;
}

int32
if_cmd_process_check_mgmt_if_ip(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    addr_t addr;
    tbl_route_if_t *p_overlap_rtif = NULL;
    char addr_str[GLB_IPV6_STR_LEN];
    int32 rc = PM_E_NONE;

    sal_memset(&addr, 0, sizeof(addr));

    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        if (1 == sal_inet_pton(AF_INET, argv[0], &addr.u.prefix4))
        {  
            addr.family = AF_INET;
        }
        else if (1 == sal_inet_pton(AF_INET6, argv[0], &addr.u.prefix6))
        {
            addr.family = AF_INET6;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[0]);
        }

        if (rt_cmd_addr_check_overlap(&addr, &p_overlap_rtif))
        {
            IFNAME_ETH2FULL(p_overlap_rtif->key.name, ifname_ext);
            CFG_CONFLICT_RET("Address %s on mgmt-if overlaps with interface %s", 
                cdb_addr_val2str(addr_str, GLB_IPV6_STR_LEN, &addr), ifname_ext);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

int32
if_cmd_process_cfg_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if        = NULL;
    tbl_kernel_if_t *pst_kernel_if  = NULL;
    tbl_kernel_if_t *p_kernel_if    = NULL;
    char            buf[MAX_CMD_STR_LEN];
    addr_ipv4_t     ip_addr;
    addr_ipv4_t     ip_addr_un;
    addr_ipv4_t     ip_addr_cmd;
    uint32          mask        = 0;
    uint32          mask_un     = 0;
    uint32          mask_len    = 0;
    uint32          addr_buf   = 0;
    uint32          has_check   = FALSE;
    int32           rc = PM_E_NONE;
    char            ifname[IFNAME_SIZE];
    char            ifname_ext[IFNAME_EXT_SIZE];

    CFG_DBG_FUNC();

    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_un, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_cmd, 0, sizeof(addr_ipv4_t));
    sal_memset(ifname, 0, sizeof(ifname));

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Num of parameters get none!");
        }
        else if (4 == argc)
        {
            sal_strncpy(ifname, argv[3], sizeof(ifname));
            p_db_if = tbl_interface_get_interface_by_name(ifname);
            if (NULL == p_db_if)
            {
                CFG_INVALID_PARAM_RET("Interface does not exist!");
            }

            if (0 == sal_strcmp(argv[0], "if_addr_cfg_check"))
            {
                if (GLB_IF_MODE_L3 != p_db_if->mode)
                {
                    CFG_INVALID_PARAM_RET("Interface %s is not L3 interface!", argv[3]);
                }

                if (p_db_if->lacp || p_db_if->lag)
                {
                    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
                    {
                        goto ip_check;
                    }

                    CFG_INVALID_PARAM_RET("Interface %s is bind to agg port!", argv[3]);
                }

ip_check:
                pst_kernel_if = tbl_kernel_if_get_by_name(argv[3]);
                if (NULL == pst_kernel_if)
                {
                    return PM_E_NONE;
                }

                if (0x0 != pst_kernel_if->ip_addr.s_addr)
                {
                    sal_memset(buf, 0, sizeof(buf));
                    CFG_INVALID_PARAM_RET("Interface %s IP address is %s",
                         argv[3], cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &pst_kernel_if->ip_addr));
                }

                addr_buf = rt_cli_get_ipv4_prefix(argv[1]);

                ip_addr.s_addr = addr_buf;
                ip_addr_un.s_addr = addr_buf;
                ip_addr_cmd.s_addr = addr_buf;
                ip_addr_cmd.s_addr = sal_htonl(ip_addr_cmd.s_addr);

                p_kernel_if = tbl_kernel_if_check_by_list_loop(&ip_addr);
                if (NULL != p_kernel_if)
                {
                    sal_memset(buf, 0, sizeof(buf));
                    sal_memset(ifname_ext, 0, sizeof(ifname_ext));
                    IFNAME_ETH2FULL(p_kernel_if->name, ifname_ext);

                    CFG_INVALID_PARAM_RET(" %s overlaps with %s",
                            cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_kernel_if->ip_addr),
                                ifname_ext);

                    has_check = TRUE;
                }

                if(IN_MULTICAST (ip_addr.s_addr)
                    || IPV4_NET127(ip_addr.s_addr)
                    || IPV4_CHECK1(ip_addr.s_addr)
                    || IPV4_CHECK2(ip_addr.s_addr)
                    || IPV4_NET0(ip_addr.s_addr))
                {

                    CFG_INVALID_PARAM_RET("Interface %s IP address is invalid!", argv[3]);
                }

                mask_len = sal_atoi(argv[2]);
                IPV4_LEN_TO_MASK(mask, mask_len);
                IPV4_LEN_TO_MASK(mask_un, mask_len);

                ip_addr.s_addr = ip_addr.s_addr & mask;
                mask_un = ~mask_un;
                ip_addr_un.s_addr = ip_addr_un.s_addr & mask_un;

                if (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
                {
                    if (32 != mask_len)
                    {
                        CFG_INVALID_PARAM_RET("Interface %s IP address is invalid!", argv[3]);
                    }

                    sal_strcpy(ifname, argv[3]);
                    rc = rt_cmd_add_if_addr_by_syscmd(ifname, &ip_addr_cmd, mask_len, TRUE);
                    
                    rt_if_indirect_ip_route_inc(p_db_if, TRUE, FALSE);
                    return PM_E_NONE;
                }

                if (32 == mask_len)
                {
                    CFG_INVALID_PARAM_RET("IP address is invalid!");
                }
                else
                {
                    if(31 == mask_len || 0x0 == ip_addr.s_addr || 0 == ip_addr_un.s_addr ||
                                ip_addr_un.s_addr == mask_un)
                    {
                        CFG_INVALID_PARAM_RET("IP address or mask is invalid!");
                    }
                }

                if (FALSE == has_check)
                {
                    p_kernel_if = tbl_kernel_if_check_by_ipmask_list_loop(&ip_addr);
                    if (NULL != p_kernel_if)
                    {
                        sal_memset(buf, 0, sizeof(buf));
                        sal_memset(ifname_ext, 0, sizeof(ifname_ext));
                        IFNAME_ETH2FULL(p_kernel_if->name, ifname_ext);
                        CFG_INVALID_PARAM_RET(" %s overlaps with %s",
                                cdb_addr_ipv4_val2str(buf, MAX_CMD_STR_LEN, &p_kernel_if->ip_addr),
                                    ifname_ext);
                    }
                }
            }

            sal_strcpy(ifname, argv[3]);
            rc = rt_cmd_add_if_addr_by_syscmd(ifname, &ip_addr_cmd, mask_len, TRUE);
            rt_if_indirect_ip_route_inc(p_db_if, TRUE, FALSE);
        }
        else if (3 == argc)
        {
            mask_len = sal_atoi(argv[1]);
            sal_strcpy(ifname, argv[2]);

            addr_buf = rt_cli_get_ipv4_prefix(argv[0]);
            ip_addr_cmd.s_addr = addr_buf;
            ip_addr_cmd.s_addr = sal_htonl(ip_addr_cmd.s_addr);
            rt_cmd_del_if_addr_by_syscmd(ifname, &ip_addr_cmd, mask_len, TRUE);

            p_db_if = tbl_interface_get_interface_by_name(ifname);
            if (NULL == p_db_if)
            {
                return PM_E_FAIL;
            }

            rt_if_indirect_ip_route_dec(p_db_if, TRUE, FALSE);
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_route(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    char            sys_cmd[128];
    int32           rc = PM_E_NONE;
    uint32          route_count = 0;

    CFG_DBG_FUNC();

    sal_memset(sys_cmd, 0, sizeof(sys_cmd));
    p_node = cdb_get_tbl(TBL_IPROUTE_NODE);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "count_show"))
            {
                route_count = rt_fea_get_route_count_global();
                rc  = _rt_cmd_get_route_count(para, p_node, &route_count);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter!");
        }

        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_route(argv[0], para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_api_mask_apply(prefix_t *prefix_cp, prefix_t *dest, uint8 prefixlen)
{
    uint32      mask = 0;

    sal_memset(prefix_cp, 0, sizeof(prefix_t));
    sal_memcpy(prefix_cp, dest, sizeof(prefix_t));
    
    if (AF_INET == dest->family)
    {
        IPV4_LEN_TO_MASK(mask, prefixlen);
        prefix_cp->u.prefix4.s_addr = sal_htonl(prefix_cp->u.prefix4.s_addr);
        prefix_cp->u.prefix4.s_addr = prefix_cp->u.prefix4.s_addr & mask;
        prefix_cp->u.prefix4.s_addr = sal_htonl(prefix_cp->u.prefix4.s_addr);
    }
    else
    {
        ip_apply_mask(prefix_cp);
    }

    return PM_E_NONE;
}

int32
route_cmd_ipv4_add_nhg(nexthop_ecmp_routed_t *nh_group, bool max_check, uint32 is_ipv4)
{
    tbl_nexthop_group_routed_t nhgroup;
    tbl_nexthop_group_routed_t* p_nhg;
    int32 rc;

    sal_memset(&nhgroup, 0, sizeof(nhgroup));

    sal_memcpy(&nhgroup.key, nh_group, sizeof(nexthop_ecmp_routed_t));
    p_nhg = tbl_nexthop_group_routed_get_nexthop_group_routed(&nhgroup.key);
    if(NULL == p_nhg)
    {
        if (is_ipv4)
        {
            if (rt_fea_ecmp_groups_count_max_check())
            {
                if (TRUE == max_check)
                {
                    log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u !", rt_fea_ecmp_count_get_max());
                    return PM_E_EXCEED_ECMP_MAX_NUM;
                }
            }

            rt_ecmp_groups_counter_inc(DEFAULT_VRF_ID);
        }
        else
        {
            if (rt_fea_v6_ecmp_groups_count_max_check())
            {
                if (TRUE == max_check)
                {
                    log_sys(M_MOD_ROUTE, E_ERROR, "The ecmp entry num is more than maximum %u !", rt_fea_v6_ecmp_count_get_max());
                    return PM_E_EXCEED_ECMP_MAX_NUM;
                }
            }

            rt_ecmp_groups_v6_counter_inc(DEFAULT_VRF_ID);
        }
        
        nhgroup.ref_cnt = 1;
        
        PM_E_RETURN(tbl_nexthop_group_routed_add_nexthop_group_routed(&nhgroup));
    }
    else
    {
        p_nhg->ref_cnt++;
        PM_E_RETURN(tbl_nexthop_group_routed_set_nexthop_group_routed_field(p_nhg, TBL_NEXTHOP_GROUP_ROUTED_FLD_REF_CNT));
    }
    
    return PM_E_NONE;

}

int32
route_cmd_ipv4_del_nhg(nexthop_ecmp_routed_t *nh_group, uint32 is_ipv4)
{
    tbl_nexthop_group_routed_t nhgroup;
    tbl_nexthop_group_routed_t* p_nhg;
    int32 rc;

    sal_memset(&nhgroup, 0, sizeof(nhgroup));

    sal_memcpy(&nhgroup.key, nh_group, sizeof(nexthop_ecmp_routed_t));
    p_nhg = tbl_nexthop_group_routed_get_nexthop_group_routed(&nhgroup.key);
    
    if(NULL == p_nhg)
    {
        return PM_E_NOT_EXIST;
    }

    /*2, delete nhg, release resource*/
    if(p_nhg->ref_cnt > 1)
    {
        p_nhg->ref_cnt --;
        PM_E_RETURN(tbl_nexthop_group_routed_set_nexthop_group_routed_field(p_nhg, TBL_NEXTHOP_GROUP_ROUTED_FLD_REF_CNT));
    }
    else if(p_nhg->ref_cnt == 1)
    {
        /*delete nexthop group*/
        if (is_ipv4)
        {
            rt_ecmp_groups_counter_dec(DEFAULT_VRF_ID);
        }
        else
        {
            rt_ecmp_groups_v6_counter_dec(DEFAULT_VRF_ID);
        }
        
        PM_E_RETURN(tbl_nexthop_group_routed_del_nexthop_group_routed(&nhgroup.key));
    }
    else
    {
        return PM_E_FAIL;
    }
    
    return PM_E_NONE;

}

#define route_cmd_ipv6_del_nhg route_cmd_ipv4_del_nhg
#define route_cmd_ipv6_add_nhg route_cmd_ipv4_add_nhg

int32
rt_is_static_route_gate_exist_in_nexthop_group_routed(tbl_static_rt_cnt_t  *pst_static_rt_cnt, prefix_t *gate)
{
    nexthop_ecmp_routed_t nh_group;
    tbl_nexthop_group_routed_t *pst_nexthop_group_routed = NULL;
    tbl_nexthop_group_routed_t *pst_nexthop_group_routed_old = NULL;

    sal_memset(&nh_group, 0x00, sizeof(nexthop_ecmp_routed_t));
    sal_memcpy(&nh_group, &pst_static_rt_cnt->nh_group, sizeof(nexthop_ecmp_routed_t));
    pst_nexthop_group_routed_old = tbl_nexthop_group_routed_get_nexthop_group_routed(&nh_group);
    if (!pst_nexthop_group_routed_old)
    {
        /*1 to ecmp 2, ecmp 2 has already exist*/
        if (nh_group.nh_num == 1)
        {
            
        }
        else
        {
            return FALSE;
        }
    }
    if (gate->family == AF_INET)
    {
        sal_memcpy(&nh_group.nh[pst_static_rt_cnt->nh_group.nh_num].u.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
        nh_group.nh[pst_static_rt_cnt->nh_group.nh_num].type = NH_TYPE_IPV4;
        nh_group.nh_num++;
    }
    else if (gate->family == AF_INET6)
    {
        sal_memcpy(&nh_group.nh[pst_static_rt_cnt->nh_group.nh_num].u.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
        nh_group.nh[pst_static_rt_cnt->nh_group.nh_num].type = NH_TYPE_IPV6;
        nh_group.nh_num++;
    }

    pst_nexthop_group_routed = tbl_nexthop_group_routed_get_nexthop_group_routed(&nh_group);
    if (pst_nexthop_group_routed)
    {
        /*the new nexthop_groute_routed already exist*/
        return TRUE;
    }
    else
    {
        if (nh_group.nh_num >= 3 && pst_nexthop_group_routed_old->ref_cnt == 1)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

int32
rt_cmd_static_rt_cnt_add(vrf_id_t vrf_id, prefix_t *dest, uint8 prefixlen, 
    uint32 distance, uint32 is_ipv4, prefix_t *gate, uint32 is_blackhole, int32 is_update_distance, uint32 old_distance, int32 *count_add_failed)
{
    tbl_static_rt_cnt_t  *pst_static_rt = NULL;
    tbl_static_rt_cnt_t  static_rt;
    prefix_t prefix_cp;
    //uint32      mask = 0;
    int32 ret = PM_E_NONE;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    /* 1. for data transform to save */
    rt_api_mask_apply(&prefix_cp, dest, prefixlen);

    /* 2. get route  */
    sal_memset(&static_rt, 0, sizeof(static_rt));
    static_rt.key.vrf_id = vrf_id;
    static_rt.key.p.family = dest->family;
    static_rt.key.p.prefixlen = prefixlen;
    static_rt.key.distance = distance;
    sal_memcpy (&static_rt.key.p, &prefix_cp, sizeof(prefix_t));

    /*ecmp*/
    pst_static_rt = tbl_static_rt_cnt_get_static_rt_cnt(&static_rt.key);
    if (NULL != pst_static_rt)
    {
        if ((dest->family == AF_INET && rt_fea_ecmp_groups_count_max_check()) || 
            (dest->family == AF_INET6 && rt_fea_v6_ecmp_groups_count_max_check()))
        {
            if (FALSE == rt_is_static_route_gate_exist_in_nexthop_group_routed(pst_static_rt, gate))
            {
                return PM_E_ECMP_RESOURCE_FULL;
            }
        }
        
        if (pst_static_rt->refer_cnt >= 2)
        {
            route_cmd_ipv4_del_nhg(&pst_static_rt->nh_group, is_ipv4);
        }
        
        if (is_ipv4)
        {
            sal_memcpy(&pst_static_rt->nh_group.nh[pst_static_rt->refer_cnt].u.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
            pst_static_rt->nh_group.nh[pst_static_rt->refer_cnt].type = NH_TYPE_IPV4;
        }
        else
        {
            sal_memcpy(&pst_static_rt->nh_group.nh[pst_static_rt->refer_cnt].u.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
            pst_static_rt->nh_group.nh[pst_static_rt->refer_cnt].type = NH_TYPE_IPV6;
        }

        pst_static_rt->nh_group.nh_num++;
        pst_static_rt->refer_cnt++;
        if (is_blackhole)
        {
            pst_static_rt->has_blackhole = TRUE;
        }
        //tbl_static_rt_cnt_set_static_rt_cnt_field(pst_static_rt, TBL_STATIC_RT_CNT_FLD_NH_GROUP);
        //tbl_static_rt_cnt_set_static_rt_cnt_field(pst_static_rt, TBL_STATIC_RT_CNT_FLD_REFER_CNT);
        //tbl_static_rt_cnt_set_static_rt_cnt_field(pst_static_rt, TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE);

        route_cmd_ipv4_add_nhg(&pst_static_rt->nh_group, TRUE, is_ipv4);
    }
    else
    {
        ret = rt_if_indirect_ip_route_inc(NULL, is_ipv4, FALSE);
        if (PM_E_NONE != ret)
        {
            if (is_update_distance)
            {
                static_rt.key.distance = old_distance;
                pst_static_rt = tbl_static_rt_cnt_get_static_rt_cnt(&static_rt.key);
                if (pst_static_rt)
                {
                    if (pst_static_rt->refer_cnt > 1)
                    {
                        return ret;
                    }
                    else if (pst_static_rt->refer_cnt == 1)
                    {
                        *count_add_failed = TRUE;
                    }
                    else
                    {
                        return PM_E_FAIL;
                    }
                }
                else
                {
                    return PM_E_FAIL;
                }
            }
            else
            {
                return ret;
            }
        }

        static_rt.key.distance = distance;
        if (is_ipv4)
        {
            sal_memcpy(&static_rt.nh_group.nh[static_rt.refer_cnt].u.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
            static_rt.nh_group.nh[static_rt.refer_cnt].type = NH_TYPE_IPV4;
            rt_static_routes_inc(DEFAULT_VRF_ID);
        }
        else
        {
            sal_memcpy(&static_rt.nh_group.nh[static_rt.refer_cnt].u.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
            static_rt.nh_group.nh[static_rt.refer_cnt].type = NH_TYPE_IPV6;
            rt_static_v6_routes_inc(DEFAULT_VRF_ID);
        }

        static_rt.nh_group.nh_num = 1;
        static_rt.refer_cnt = 1;
        if (is_blackhole)
        {
            static_rt.has_blackhole = TRUE;
        }

        //rt_if_indirect_ip_route_inc(NULL, is_ipv4, FALSE);
        
        tbl_static_rt_cnt_add_static_rt_cnt_sync(&static_rt, FALSE);
    }

    return PM_E_NONE;
}

int32
rt_cmd_static_rt_cnt_del(vrf_id_t vrf_id, prefix_t *dest, uint8 prefixlen, uint32 distance, uint32 is_ipv4, prefix_t *gate, int32 count_add_failed)
{
    tbl_static_rt_cnt_t  *pst_static_rt = NULL;
    tbl_static_rt_cnt_t  static_rt;
    prefix_t prefix_cp;
    //uint32      mask = 0;
    uint32 i = 0;
    addr_ipv4_t tmp_addr;
    uint32 is_blackhole = FALSE;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    
    if (gate->family == AF_INET)
    {
        sal_memset(&tmp_addr, 0x00, sizeof(addr_ipv4_t));
        if (0 == sal_memcmp(&gate->u.prefix4, &tmp_addr, sizeof(addr_ipv4_t)))
        {
            is_blackhole = TRUE;
        }
    }

    /* 1. for data transform to save */
    rt_api_mask_apply(&prefix_cp, dest, prefixlen);

    /* 2. get route  */
    sal_memset(&static_rt, 0, sizeof(static_rt));
    static_rt.key.vrf_id = vrf_id;
    static_rt.key.p.family = dest->family;
    static_rt.key.p.prefixlen = prefixlen;
    static_rt.key.distance = distance;
    sal_memcpy (&static_rt.key.p, &prefix_cp, sizeof(prefix_t));

    pst_static_rt = tbl_static_rt_cnt_get_static_rt_cnt(&static_rt.key);
    if (NULL != pst_static_rt)
    {
        if (1 < pst_static_rt->refer_cnt)
        {
            route_cmd_ipv4_del_nhg(&pst_static_rt->nh_group, is_ipv4);
            
            for(i = 0; i < pst_static_rt->refer_cnt && i < ECMP_MAX_MEMBER; i++)
            {
                if ((is_ipv4 && 0 == sal_memcmp(&pst_static_rt->nh_group.nh[i].u.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t))) ||
                    (!is_ipv4 && 0 == sal_memcmp(&pst_static_rt->nh_group.nh[i].u.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t))))
                {
                    sal_memset(&pst_static_rt->nh_group.nh[i], 0x00, sizeof(nexthop_key_t));

                    #if 0
                    if (i == pst_static_rt->refer_cnt - 1)
                    {
                        //do nothing
                    }
                    else if (i < pst_static_rt->refer_cnt - 1)
                    {
                        sal_memcpy(&pst_static_rt->nh_group.nh[i], &pst_static_rt->nh_group.nh[pst_static_rt->refer_cnt - 1], sizeof(nexthop_key_t));
                        break;
                    }
                    #else
                    uint32 j = 0;
                    for (j = i; j  < pst_static_rt->refer_cnt && j < ECMP_MAX_MEMBER - 1 && i < ECMP_MAX_MEMBER; j++)
                    {
                        sal_memcpy(&pst_static_rt->nh_group.nh[j], &pst_static_rt->nh_group.nh[j + 1], sizeof(nexthop_key_t));
                    }
                    
                    sal_memset(&pst_static_rt->nh_group.nh[j], 0x00, sizeof(nexthop_key_t));
                    break;
                    #endif
                }
            }
            
            pst_static_rt->refer_cnt--;
            pst_static_rt->nh_group.nh_num--;
            if (is_blackhole)
            {
                pst_static_rt->has_blackhole = FALSE;
            }
            //tbl_static_rt_cnt_set_static_rt_cnt_field(pst_static_rt, TBL_STATIC_RT_CNT_FLD_REFER_CNT);
            //tbl_static_rt_cnt_set_static_rt_cnt_field(pst_static_rt, TBL_STATIC_RT_CNT_FLD_NH_GROUP);
            //tbl_static_rt_cnt_set_static_rt_cnt_field(pst_static_rt, TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE);

            if (pst_static_rt->refer_cnt >= 2)
            {
                route_cmd_ipv4_add_nhg(&pst_static_rt->nh_group, TRUE, is_ipv4);
            }
        }
        else/*refer_cnt =1, delete it*/
        {
            if (is_ipv4)
            {
                rt_static_routes_dec(DEFAULT_VRF_ID);
            }
            else
            {
                rt_static_v6_routes_dec(DEFAULT_VRF_ID);
            }

            if (!count_add_failed)
            {
                rt_if_indirect_ip_route_dec(NULL, is_ipv4, FALSE);
            }
            
            tbl_static_rt_cnt_del_static_rt_cnt_sync(&pst_static_rt->key, FALSE);
        }
    }
    
    return PM_E_NONE;
}

int32
rt_cmd_reset_ospf_process(void)
{
#ifndef TAPPRODUCT
    char        cmd_str[128];

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_sprintf(cmd_str, "reset ip ospf process");
    rt_vty_exec_cmd(cmd_str);
#endif

    return PM_E_NONE;
}

/*if gate is same with if addr, will not commit to frr and fea*/
//rt_fea_ipv6_gate_check(addr_ipv6_t *gate)
int32
rt_fea_ipv6_gate_check_same_with_if_addr(addr_ipv6_t *gate)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_v6_t      *p_tmp_conn = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_route_if_t *p_db_route_if = NULL;
    
    if (NULL == p_master)
    {
        return FALSE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_route_if, listnode, next)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_db_route_if->key.name);
        if (NULL == p_db_if)
        {
            continue;
        }
        
        CTCLIB_SLIST_LOOP(p_db_route_if->ifc_ipv6.obj_list, p_tmp_conn, listnode)
        {
            if (0 == sal_memcmp(&p_tmp_conn->key.address.u.prefix6, gate, sizeof (addr_ipv6_t)))
            {
                return TRUE;
            }
        }
    }
        
    return FALSE;
}

int32
rt_cmd_add_static_route_by_syscmd(prefix_t *dest, prefix_t *gate,
                        uint8 prefixlen, uint32 distance, uint32 ifindex, uint32 is_blackhole)
{
    char        buf[CMD_BUF_64];
    char        buf2[CMD_BUF_64];
    char        cmd_str[256];

    char ifname[IFNAME_EXT_SIZE] = {0};

    /*if gate is same with if addr, will not commit to frr and fea*/
    if (AF_INET6 == dest->family && TRUE == rt_fea_ipv6_gate_check_same_with_if_addr(&gate->u.prefix6))
    {
        //return PM_E_NONE;
    }
    
    if (0 != ifindex)
    {
        if (0 != tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_EXT_SIZE))
        {
            MY_PRINTF("get ifname by ifindex %u failed", ifindex);
        }
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    dest->prefixlen = prefixlen;

    prefix_val2str(buf, CMD_BUF_64, dest, TRUE);
    prefix_val2str(buf2, CMD_BUF_64, gate, FALSE);

    if (is_blackhole)
    {
        sal_sprintf(cmd_str, "ip route %s null0 %d",
            prefix_val2str(buf, CMD_BUF_256, dest, TRUE), distance);
    }
    else
    {
        if (1 != distance)
        {
            sal_sprintf(cmd_str, "ip%s route %s %s %s %d",
                        (dest->family == AF_INET) ? "" : "v6",
                        buf,
                        buf2,
                        ifname,
                        distance);
        }
        else
        {
            sal_sprintf(cmd_str, "ip%s route %s %s %s",
                        (dest->family == AF_INET) ? "" : "v6",
                        buf,
                        buf2,
                        ifname);
        }
    }
    rt_vty_config_cmd(cmd_str);
        
    return PM_E_NONE;
}

int32
rt_delete_recursive_static_route(prefix_t *dest, prefix_t *gate)
{
    int32  rc   = PM_E_NONE;
    char   cmd_str[CMD_BUF_256];
    char   dest_str[CMD_BUF_32];
    tbl_iproute_node_key_t fea_rt;
    tbl_iproute_node_t  *p_fea_rt = NULL;
    int32  recursive = FALSE;
    int32  i = 0;

    sal_memset(cmd_str, 0, sizeof(cmd_str));
    sal_memset(dest_str, 0, sizeof(dest_str));
    cdb_addr_ipv4_val2str(dest_str, CMD_BUF_32, &(dest->u.prefix4));

    /* process for recursive route */
    sal_memset(&fea_rt, 0, sizeof(fea_rt));
    fea_rt.vrf_id = 0;
    sal_memcpy(&fea_rt.p.u.prefix4, &(dest->u.prefix4), sizeof(addr_ipv4_t));
    fea_rt.p.prefixlen = dest->prefixlen;
    fea_rt.p.family = dest->family;
    p_fea_rt = tbl_iproute_node_get_iproute_node(&fea_rt);
    if (p_fea_rt)
    {
        if (!p_fea_rt->multipath)
        {
            /*for bug 50453 by chenc, 2018-12-20*/
            if (sal_memcmp(&(gate->u.prefix4), &p_fea_rt->nh_key.u.ipv4, sizeof(addr_ipv4_t)) && p_fea_rt->nh_key.type != NH_TYPE_LOCAL)
            {
                recursive = TRUE;
            }
        }
        else
        {
            recursive = TRUE;
            for (i = 0; i < p_fea_rt->nh_group.nh_num; ++i)
            {
                /*for bug 50453 by chenc, 2018-12-20*/
                if (!sal_memcmp(&(gate->u.prefix4), &p_fea_rt->nh_group.nh[i].u.ipv4, sizeof(addr_ipv4_t)) || p_fea_rt->nh_key.type == NH_TYPE_LOCAL)
                {
                    recursive = FALSE;
                    break;
                }
            }
        }
    }

    if (TRUE == recursive)
    {
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sprintf(cmd_str, "ip route del %s/%d", dest_str, dest->prefixlen);
        sal_system(cmd_str);
    }

    return rc;
}

int32
rt_cmd_del_static_route_by_syscmd(prefix_t *dest, prefix_t *gate, uint8 prefixlen, uint32 ifindex, uint32 is_blackhole)
{
    char        buf[CMD_BUF_256];
    char        buf2[CMD_BUF_256];
    char        cmd_str[256];

    char ifname[IFNAME_EXT_SIZE] = {0};

    /*if gate is same with if addr, will not commit to frr and fea*/
    if (AF_INET6 == dest->family && TRUE == rt_fea_ipv6_gate_check_same_with_if_addr(&gate->u.prefix6))
    {
        //return PM_E_NONE;
    }
    
    if (0 != ifindex)
    {
        if (0 != tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_EXT_SIZE))
        {
            MY_PRINTF("get ifname by ifindex %u failed", ifindex);
        }
    }

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(cmd_str, 0, sizeof(cmd_str));

    dest->prefixlen = prefixlen;

    if (is_blackhole)
    {
        sal_sprintf(cmd_str, "no ip route %s null0",
            prefix_val2str(buf, CMD_BUF_256, dest, TRUE));
    }
    else
    {
        sal_sprintf(cmd_str, "no ip%s route %s %s %s",
                (dest->family == AF_INET) ? "" : "v6",
                prefix_val2str(buf, CMD_BUF_256, dest, TRUE),
                prefix_val2str(buf2, CMD_BUF_256, gate, FALSE),
                ifname);
    }
    rt_vty_config_cmd(cmd_str);

    if (dest->family == AF_INET6)
    {
        rt_fea_ipv6_del_from_iproute_node_add_fail(0 , &dest->u.prefix6, prefixlen, &gate->u.prefix6, NULL, 0);
    }
    else
    {
        rt_delete_recursive_static_route(dest, gate);
        rt_fea_ipv4_del_from_iproute_node_add_fail(0 , &dest->u.prefix4, prefixlen, &gate->u.prefix4, NULL, 0);
        rt_fea_ipv4_del_from_iproute_node_add_fail_count(0 , &dest->u.prefix4, prefixlen, &gate->u.prefix4, NULL, 0);
    }

    return PM_E_NONE;
}


/*  for tbl, skip_key = TRUE;
    for global, skip_key = FALSE*/
void
cfg_parser_cmd_args_new(char **argv, int32 argc, cfg_cmd_para_t *para, 
                              new_cmd_args_key_value_t key_value[], int32 key_num, bool skip_key)
{
    int32 i = 0;
    int32 j = 0;
    
    for (skip_key ? (i = 1): (i = 0); i < argc; i++)
    {
        for (j = 0; j < key_num; j++)
        {
        /* match key name */
            if (0 == sal_strcmp(argv[i], key_value[j].key_name))
            {
                i++;
                if (NULL != argv[i])
                {
                    sal_strcpy(key_value[j].key_value, argv[i]);
                }
                break;
            }
        }
    }

    return;
}

int32
rt_is_static_rt_ecmp_member_exceed(vrf_id_t vrf_id, prefix_t *dest, prefix_t *gate, uint8 prefixlen, uint32 distance)
{
    tbl_static_rt_cnt_t  *pst_static_rt_cnt = NULL;
    tbl_static_rt_cnt_t  static_rt_cnt;
    prefix_t prefix_cp;
    int32 ecmp_member_num = 0;

    tbl_static_route_cfg_t  *pst_static_route_cfg = NULL;
    tbl_static_route_cfg_t  static_route_cfg;

    if (NULL == dest)
    {
        return PM_E_FAIL;
    }

    ecmp_member_num = (dest->family == AF_INET) ?  rt_fea_ecmp_member_count_get_max() : rt_fea_v6_ecmp_member_count_get_max();

    /* 1. for data transform to save */
    rt_api_mask_apply(&prefix_cp, dest, prefixlen);
    
    /* 2. get static_rt_cnt */
    sal_memset(&static_rt_cnt, 0, sizeof(static_rt_cnt));
    static_rt_cnt.key.vrf_id = vrf_id;
    static_rt_cnt.key.p.family = dest->family;
    static_rt_cnt.key.p.prefixlen = prefixlen;
    static_rt_cnt.key.distance = distance;
    sal_memcpy(&static_rt_cnt.key.p, &prefix_cp, sizeof(prefix_t));

    /* 3. get static_route_cfg  */
    sal_memset(&static_route_cfg, 0, sizeof(static_route_cfg));
    static_route_cfg.key.vrf_id = vrf_id;
    static_route_cfg.key.p.family = dest->family;
    static_route_cfg.key.p.prefixlen = prefixlen;

    if (AF_INET == dest->family)
    {
        sal_memcpy(&static_route_cfg.key.nh_addr.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memcpy(&static_route_cfg.key.nh_addr.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
    }
    sal_memcpy (&static_route_cfg.key.p, &prefix_cp, sizeof(prefix_t));

    pst_static_route_cfg = tbl_static_route_cfg_get_static_route_cfg(&static_route_cfg.key);

    pst_static_rt_cnt = tbl_static_rt_cnt_get_static_rt_cnt(&static_rt_cnt.key);
    if (NULL != pst_static_rt_cnt)
    {
        /*ecmp already max*/
        if (pst_static_rt_cnt->refer_cnt >= ecmp_member_num)
        {
            /*for config the same route, not print err msg*/
            if (NULL == pst_static_route_cfg)
            {
                if (!(pst_static_rt_cnt->has_blackhole && pst_static_rt_cnt->refer_cnt == ecmp_member_num))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

int32
rt_cmd_static_route_data_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_iproute_node_t      *pst_iproute_node   = NULL;
    tbl_iproute_node_key_t  rn_key;
    addr_ipv4_t             ip_addr;
    addr_ipv4_t             ip_addr_spcheck;
    addr_ipv4_t             ip_addr_gw;
    addr_ipv4_t             ip_gw_cp;
    uint32                  mask     = 0;
    uint32                  mask_spcheck     = 0;
    uint32                  buf     = 0;
    uint32                  buf2    = 0;
    uint32                  i       = 0;
    uint32                  nh_ret  = 0;
    char    ip_addr_str[GLB_IPV4_STR_LEN];
    uint8 prefixlen = 0;
    int32 key_num = 1;
    int32 ecmp_member_num = 0;
    uint32 is_ipv4 = TRUE;
    int32 rc = PM_E_NONE;
    uint32 is_blackhole = FALSE;

    addr_ipv6_t             ipv6_addr;
    addr_ipv6_t             ipv6_addr_spcheck;
    addr_ipv6_t             ipv6_addr_gw;
    addr_ipv6_t             ipv6_gw_cp;
    char    ipv6_addr_str[GLB_IPV6_STR_LEN] = {0};

    new_cmd_args_key_value_t key_value;

    if (0 == sal_strcmp(argv[0], "ip"))
    {
        is_ipv4 = TRUE;
    }
    else if (0 == sal_strcmp(argv[0], "ipv6"))
    {
        is_ipv4 = FALSE;
    }
    else
    {
        MY_PRINTF("err static route cfg");
    }

    if (!sal_strcmp(argv[3], "null"))
    {
        is_blackhole = TRUE;
    }

    sal_memset(&key_value, 0, sizeof(key_value));
    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_gw, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_gw_cp, 0, sizeof(addr_ipv4_t));
    sal_memset(&ip_addr_spcheck, 0, sizeof(addr_ipv4_t));
    sal_memset(ip_addr_str, 0, sizeof(ip_addr_str));

    sal_memset(&ipv6_addr, 0, sizeof(addr_ipv6_t));
    sal_memset(&ipv6_addr_gw, 0, sizeof(addr_ipv6_t));
    sal_memset(&ipv6_gw_cp, 0, sizeof(addr_ipv6_t));
    sal_memset(&ipv6_addr_spcheck, 0, sizeof(addr_ipv6_t));
    sal_memset(ipv6_addr_str, 0, sizeof(ipv6_addr_str));

    if (is_ipv4)
    {
        sal_strcpy(key_value.key_name, "ipgw");
    }
    else
    {
        sal_strcpy(key_value.key_name, "ipv6_gw");
    }

    cfg_parser_cmd_args_new(argv, argc, para, &key_value, key_num, FALSE);
    if (0 != strlen(key_value.key_value))
    {
        if (is_ipv4)
        {
            buf2 = rt_cli_get_ipv4_prefix(key_value.key_value);
            ip_addr_gw.s_addr = buf2;
        }
        else
        {
            CFG_PARSER_IPV6_ADDR(key_value.key_value, &ipv6_addr_gw, para->p_rs);
        }
    }

    if (is_ipv4)
    {
        if(IPV4_NET127(ip_addr_gw.s_addr)
            || IPV4_CHECK2(ip_addr_gw.s_addr)
            || (IPV4_CHECK3(ip_addr_gw.s_addr) && FALSE == is_blackhole)
            || (IPV4_NET0(ip_addr_gw.s_addr) && FALSE == is_blackhole))
        {
            CFG_INVALID_PARAM_RET("Please check the configuation of static route gateway!");
        }
    }
    else
    {
        /*ipv6 address gw check*/
        // TODO: cc
        #if 0
        if(IPV4_NET127(ip_addr_gw.s_addr)
            || IPV4_CHECK2(ip_addr_gw.s_addr)
            || IPV4_CHECK3(ip_addr_gw.s_addr)
            || IPV4_NET0(ip_addr_gw.s_addr))
        {
            CFG_INVALID_PARAM_RET("Please check the configuation of static route gateway!");
        }
        #endif
    }

    if (is_ipv4)
    {
        cfg_parser_cmd_args_ip_masklen(argv, argc, para, "ip", ip_addr_str, &prefixlen, FALSE);
        if (0 != sal_strlen(ip_addr_str))
        {
            buf = rt_cli_get_ipv4_prefix(ip_addr_str);
            ip_addr.s_addr = buf;
            ip_addr_spcheck.s_addr = buf;

            if (0 == ip_addr.s_addr && 0 == prefixlen)
            {
                goto out;
            }
        
            IPV4_LEN_TO_MASK(mask, prefixlen);
            ip_addr.s_addr = ip_addr.s_addr & mask;

            if(IN_MULTICAST(ip_addr.s_addr)
                || IPV4_NET127(ip_addr.s_addr)
                || IPV4_CHECK1(ip_addr.s_addr)
                || IPV4_CHECK2(ip_addr.s_addr))
            {
                CFG_INVALID_PARAM_RET("Please check the configuation of static route prefix!");
            }

            /* First 8-bit should not be 0. only when 32-bit mask and mask length < 8, it can equal to 0
             0.0.0.0/7 and 0.0.0.0/32 both are right
             0.3.2.2/32 and 0.12.2.5/9 both are wrong */
            IPV4_LEN_TO_MASK(mask_spcheck, 8);
            ip_addr_spcheck.s_addr = ip_addr_spcheck.s_addr & mask_spcheck;
            if (0 == ip_addr_spcheck.s_addr)
            {
                if (prefixlen < 8 || 32 == prefixlen)
                {
                    goto out;
                }
                else
                {
                    /* for the srs when change happens */
                    //CFG_INVALID_PARAM_RET("Please check the configuation of static route entry prefix!");
                }
            }
        }
    }
    else
    {
        cfg_parser_cmd_args_ip_masklen(argv, argc, para, "ipv6", ipv6_addr_str, &prefixlen, FALSE);
        if (0 != sal_strlen(ipv6_addr_str))
        {
            CFG_PARSER_IPV6_ADDR(ipv6_addr_str, &ipv6_addr, para->p_rs);
            // TODO:cc
            
        }
    }

out:
    sal_memset(&rn_key, 0, sizeof(rn_key));

    rn_key.vrf_id = 0;
    rn_key.p.family = is_ipv4 ? AF_INET : AF_INET6;
    rn_key.p.prefixlen = prefixlen;
    if (is_ipv4)
    {
        ip_addr.s_addr = sal_htonl(ip_addr.s_addr);
        sal_memcpy (&rn_key.p.u.prefix4, &ip_addr, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memcpy (&rn_key.p.u.prefix6, &ipv6_addr, sizeof(addr_ipv6_t));
    }
    
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&rn_key);
    if (NULL == pst_iproute_node)
    {
        return PM_E_NOT_EXIST;
    }

    if (is_ipv4)
    {
        ip_gw_cp.s_addr = sal_htonl(ip_addr_gw.s_addr);
        ecmp_member_num = rt_fea_ecmp_member_count_get_max();
    }
    else
    {
        sal_memcpy(&ipv6_gw_cp, &ipv6_addr_gw, sizeof(addr_ipv6_t));
        ecmp_member_num = rt_fea_v6_ecmp_member_count_get_max();
    }

    if ((ecmp_member_num - 1) < pst_iproute_node->nh_group.nh_num)
    {
        for (i = 0; i < pst_iproute_node->nh_group.nh_num; i++)
        {
            if (is_ipv4 && pst_iproute_node->nh_group.nh[i].u.ipv4.s_addr == ip_gw_cp.s_addr)
            {
                nh_ret = 1;
            }

            if (!is_ipv4 && 0 == sal_memcmp(&pst_iproute_node->nh_group.nh[i].u.ipv6, &ipv6_gw_cp, sizeof(addr_ipv6_t)))
            {
                nh_ret = 1;
            }
        }

        if (!nh_ret)
        {
            log_sys(M_MOD_ROUTE, E_ERROR, "The route entry's nexthop members should not exceed maximum %d!", ecmp_member_num);
            return PM_E_RESOURCE_FULL;
        }
    }

    return PM_E_NONE;
}

int32
rt_cmd_static_route_data_transform(char **argv, int32 argc, cfg_cmd_para_t *para,
                    vrf_id_t *vrf_id, prefix_t *dest, prefix_t *gate,
                    uint8 *prefixlen, uint32 *distance, uint16 *track_id, uint32 *ifindex, uint32 is_blackhole)
{
    char    buf[64];
    char    buf2[64];
    char    ip_addr[GLB_IPV4_STR_LEN];
    char    ipv6_addr[GLB_IPV6_STR_LEN];
    int32   rc = PM_E_NONE;
    new_cmd_args_key_value_t key_value[3];
    int32 key_num = 3;

    *distance = 1;

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(ip_addr, 0, sizeof(ip_addr));
    sal_memset(ipv6_addr, 0, sizeof(ipv6_addr));
    sal_memset(key_value, 0, sizeof(key_value));

    if (AF_INET == dest->family && !is_blackhole)
    {
        sal_strcpy(key_value[0].key_name, "ipgw");
        sal_strcpy(key_value[1].key_name, "distance");
        sal_strcpy(key_value[2].key_name, "track_id");
    }
    else if (AF_INET == dest->family && is_blackhole)
    {
        sal_strcpy(key_value[0].key_name, "null");
        sal_strcpy(key_value[1].key_name, "distance");
        sal_strcpy(key_value[2].key_name, "track_id");
    }
    else
    {
        sal_strcpy(key_value[0].key_name, "ipv6_gw");
        sal_strcpy(key_value[1].key_name, "distance");
        sal_strcpy(key_value[2].key_name, "interface");
    }

    vrf_id = DEFAULT_VRF_ID;

    if (AF_INET == dest->family)
    {
        cfg_parser_cmd_args_ip_masklen(argv, argc, para, 
                                  "ip", ip_addr, prefixlen, FALSE);
        if (0 != sal_strlen(ip_addr))
        {
            /*get ip*/
            rc = sal_inet_pton(AF_INET, ip_addr, buf);
            if (!rc)
            {
                CFG_INVALID_PARAM_RET("Invalid cli IP address");
            }

            sal_memcpy(&dest->u.prefix4, buf, sizeof(addr_ipv4_t));
            dest->prefixlen = *prefixlen;
        }
    }
    else
    {
        cfg_parser_cmd_args_ip_masklen(argv, argc, para, 
                                  "ipv6", ipv6_addr, prefixlen, FALSE);
        if (0 != sal_strlen(ipv6_addr))
        {
            CFG_PARSER_IPV6_ADDR(ipv6_addr, &dest->u.prefix6, para->p_rs);
        }
        dest->prefixlen = *prefixlen;
    }

    cfg_parser_cmd_args_new(argv, argc, para, key_value, key_num, FALSE);
    /*get gateway*/
    if (AF_INET == dest->family && !is_blackhole)
    {
        if (0 != strlen(key_value[0].key_value))
        {
            rc = sal_inet_pton(AF_INET, key_value[0].key_value, buf2);
            if (!rc)
            {
                CFG_INVALID_PARAM_RET("Invalid cli IP address");
            }
            sal_memcpy(&gate->u.prefix4, buf2, sizeof(addr_ipv4_t));
        }
    }
    else if (AF_INET == dest->family && is_blackhole)
    {
        
    }
    else
    {
        if (0 != strlen(key_value[0].key_value))
        {
            CFG_PARSER_IPV6_ADDR(key_value[0].key_value, &gate->u.prefix6, para->p_rs);
        }
    }

    /*get distance*/
    if (0 != strlen(key_value[1].key_value))
    {
        *distance = sal_atoi(key_value[1].key_value);
    }

    if (AF_INET == dest->family)
    {
        /*get track id*/
        if (0 != strlen(key_value[2].key_value))
        {
            PM_CFG_GET_UINT(*track_id, key_value[2].key_value, para->p_rs);
        }
    }
    else
    {
        /*get interface*/
        if (0 != strlen(key_value[2].key_value))
        {
            *ifindex = tbl_interface_get_ifindex_by_name(key_value[2].key_value);
        }
    }

    return PM_E_NONE;
}

/*is prefix_cp static route exist ?*/
int32
rt_is_static_route_dest_exist(vrf_id_t vrf_id, uint8 prefixlen, prefix_t *prefix_cp)
{
    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;

    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    /* 1. get route  */
    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = prefix_cp->family;
    static_route.key.p.prefixlen = prefixlen;
    sal_memcpy (&static_route.key.p, prefix_cp, sizeof(prefix_t));

    CTCLIB_SLIST_LOOP(p_master->rst_cfg_list, p_db_rst_cfg, listnode)
    {
        if (static_route.key.vrf_id == p_db_rst_cfg->key.vrf_id 
            && !sal_memcmp(&static_route.key.p, &p_db_rst_cfg->key.p, sizeof(prefix_t)))
        {
            if (AF_INET == prefix_cp->family)
            {
                sal_memcpy(&static_route.key.nh_addr.ipv4, &p_db_rst_cfg->key.nh_addr.ipv4, sizeof(addr_ipv4_t));
            }
            else
            {
                sal_memcpy(&static_route.key.nh_addr.ipv6, &p_db_rst_cfg->key.nh_addr.ipv6, sizeof(addr_ipv6_t));
            }
            
            pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
            if (NULL != pst_static_route)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

int32
rt_is_static_route_gate_exist_in_iproute_node(tbl_iproute_node_t  *pst_iproute_node, prefix_t *gate)
{
    uint32 index = 0;
    uint32 ecmp_member_num = 0;

    ecmp_member_num = 
        gate->family == AF_INET ? rt_fea_ecmp_member_count_get_max() : rt_fea_v6_ecmp_member_count_get_max();
    
    if (pst_iproute_node->multipath)
    {
        if (AF_INET == gate->family)
        {
            for(index = 0; index < ecmp_member_num && index< pst_iproute_node->nh_group.nh_num; index++)
            {
                if (0 == sal_memcmp(&pst_iproute_node->nh_group.nh[index].u.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t)))
                {
                    return TRUE;
                }
            }
        }
        else
        {
            for(index = 0; index < ecmp_member_num && index< pst_iproute_node->nh_group.nh_num; index++)
            {
                if (0 == sal_memcmp(&pst_iproute_node->nh_group.nh[index].u.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t)))
                {
                    return TRUE;
                }
            }
        }
    }
    else
    {
        if ((AF_INET == gate->family && (0 == sal_memcmp(&pst_iproute_node->nh_key.u.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t))))
            || (AF_INET6 == gate->family && (0 == sal_memcmp(&pst_iproute_node->nh_key.u.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t)))))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
rt_cmd_check_ecmp_resource_exceed(vrf_id_t vrf_id, prefix_t *dest, uint8 prefixlen, prefix_t *gate)
{
    tbl_iproute_node_t  *pst_iproute_node = NULL;
    tbl_iproute_node_t  iproute_node;

    sal_memset(&iproute_node, 0, sizeof(iproute_node));
    iproute_node.key.vrf_id = vrf_id;
    iproute_node.key.p.family = dest->family;
    iproute_node.key.p.prefixlen = prefixlen;
    sal_memcpy (&iproute_node.key.p, dest, sizeof(prefix_t));
    pst_iproute_node = tbl_iproute_node_get_iproute_node(&iproute_node.key);
    /*already exist, check if gate is already in nexthop or nexthop group*/
    if (pst_iproute_node)
    {
        if (TRUE == rt_is_static_route_gate_exist_in_iproute_node(pst_iproute_node, gate))
        {
            return FALSE;
        }

        if (pst_iproute_node->multipath)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    /*not exist*/
    else
    {
        return FALSE;
    }
}

int32
rt_cmd_create_static_route(vrf_id_t vrf_id, cfg_cmd_para_t *para, prefix_t *dest, prefix_t *gate, 
                           uint8 prefixlen, uint32 distance, uint16 track_id, uint32 ifindex, uint16 *is_add, 
                           int32 *is_update_distance, uint32 *is_already_exist, uint32 is_blackhole, uint32 dhcp_client)
{
    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;
    prefix_t prefix_cp;
    int32       rc = PM_E_NONE;
    int32 is_update_distance_tmp = FALSE;
    int32 count_add_failed = FALSE;

    if (NULL == dest || NULL == gate)
    {
        return PM_E_FAIL;
    }

    /* 1. for data transform to save */
    rt_api_mask_apply(&prefix_cp, dest, prefixlen);


    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = gate->family;
    static_route.key.p.prefixlen = prefixlen;
   
    if (AF_INET == gate->family)
    {
        sal_memcpy(&static_route.key.nh_addr.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memcpy(&static_route.key.nh_addr.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
        static_route.key.ifindex_v6 = ifindex;
    }
    sal_memcpy (&static_route.key.p, &prefix_cp, sizeof(prefix_t));

    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
    if (NULL != pst_static_route && pst_static_route->distance != distance)
    {
        is_update_distance_tmp = TRUE;
    }
    
    /*dest ip cannot same with nexthop*/
    /*ipv6 route fec0::1:10/128 fec0::1:10 is invalid*/
    if (AF_INET6 == dest->family)
    {
        if (0 == sal_memcmp(&gate->u.prefix6, &prefix_cp.u.prefix6, sizeof(addr_ipv6_t)))
        {
            return PM_E_ROUTE_DEST_SAME_WITH_NEXTHOP;
        }
    }

    if ((AF_INET == dest->family && rt_static_route_count_max_check()) 
        || (AF_INET6 == dest->family && rt_static_v6_route_count_max_check()))
    {
        /*fix bug 44061, if exist ,add ecmp*/
        if (TRUE != rt_is_static_route_dest_exist(vrf_id, prefixlen, &prefix_cp))
        {
            return PM_E_RESOURCE_FULL;
        }
    }

    if (AF_INET6 == dest->family)
    {
        if (rt_indirect_ipv6_route_count_max_check(1))
        {
            if (TRUE != rt_is_static_route_dest_exist(vrf_id, prefixlen, &prefix_cp))
            {
                if (!is_update_distance_tmp)
                {
                    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
                    logid_sys(LOG_ROUTE_6_INDIRECT_ROUTE_OVERFLOW, p_rt_glb->current_indirect_ipv6_route_num, 1, rt_fea_indirect_route_v6_count_get_max());
                    return PM_E_ROUTE_INDIRECT_ROUTE_FULL;
                    //CFG_CONFLICT_RET("ipv6 exceed max %u", rt_fea_indirect_route_v6_count_get_max());
                }
            }
        }
    }
    else
    {
        if (rt_indirect_ipv4_route_count_max_check(1))
        {
            if (TRUE != rt_is_static_route_dest_exist(vrf_id, prefixlen, &prefix_cp))
            {
                if (!is_update_distance_tmp)
                {
                    return PM_E_ROUTE_INDIRECT_ROUTE_FULL;
                }
            }
        }
    }

    /*ecmp over*/
    if (rt_fea_v6_ecmp_count_max_check() || rt_fea_ecmp_count_max_check())
    {
        if (rt_cmd_check_ecmp_resource_exceed(vrf_id, dest, prefixlen, gate))
        {
            //return PM_E_ECMP_RESOURCE_FULL;
        }
    }

    /*host routes check*/
    if (AF_INET6 == dest->family)
    {
        tbl_nd_fib_t        nd_fib;
        tbl_nd_fib_t        *p_nd_fib = NULL;
        tbl_nd_fib_master_t   *p_nd_fib_master = NULL;
        p_nd_fib_master = tbl_nd_fib_get_master();
        if (p_nd_fib_master)
        {
            sal_memset(&nd_fib, 0, sizeof(tbl_nd_fib_t));
            nd_fib_make_key(&nd_fib.key, &gate->u.prefix6, NULL);
            p_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib.key);
            if (NULL == p_nd_fib)
            {
                if (p_nd_fib_master->neighbor_hash->count >= nd_api_count_get_max())
                {
                    //ND_LOG_ERR("Create nd entry failure because IPv6 neighbor cache overflow\n");
                    //return PM_E_ROUTE_HOST_ROUTE_FULL;
                }
            }
        }
    }
    else
    {
        if (!is_blackhole)
        {
            tbl_arp_t       *ae_p = NULL;
            ae_p = arp_db_entry_lookup(&gate->u.prefix4);
            if (ae_p == NULL)
            {
                if (arp_db_cache_overflow())
                {
                    arp_debug_cache_overflow();
                    return PM_E_ROUTE_HOST_ROUTE_FULL;
                }
            }
        }
    }

    /* 2. get route  */
    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = gate->family;
    static_route.key.p.prefixlen = prefixlen;
   
    if (AF_INET == gate->family)
    {
        sal_memcpy(&static_route.key.nh_addr.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memcpy(&static_route.key.nh_addr.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
        static_route.key.ifindex_v6 = ifindex;
    }
    sal_memcpy (&static_route.key.p, &prefix_cp, sizeof(prefix_t));

    static_route.distance = distance;
    if (AF_INET6 == dest->family)
    {
        static_route.nexthop_type_v6 = (ifindex != 0) ? NEXTHOP_TYPE_IPV6_IFINDEX : NEXTHOP_TYPE_IPV6;
    }
    static_route.dhcp_client = dhcp_client;
     
    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
    /* modified by liwh for bug 50196, 2019-01-07 */
    if (pst_static_route && ((0 != dhcp_client) || (0 != pst_static_route->dhcp_client)))   
    {
        /* dhcp ip route shouldn't update static ip route */
        if ((0 != dhcp_client) || (0 == pst_static_route->dhcp_client))
        {
            *is_already_exist = TRUE;
            return rc;
        }
        /* clear dhcp_client bit, static ip route can update dhcp ip route */
        else if ((0 == dhcp_client) || (0 != pst_static_route->dhcp_client))
        {
            pst_static_route->dhcp_client = 0;
            rc = tbl_static_route_cfg_set_static_route_cfg_field(pst_static_route, TBL_STATIC_ROUTE_CFG_FLD_DHCP_CLIENT);
        }
        /* dhcp ip route update dhcp ip route, no any process */
        else if ((0 != dhcp_client) || (0 != pst_static_route->dhcp_client))
        {
                
        }
    }
    /* liwh end */
    
    if (NULL != pst_static_route && pst_static_route->distance != distance)
    {
        tbl_static_rt_cnt_t  *pst_static_rt = NULL;
        tbl_static_rt_cnt_t  static_rt;
        prefix_t prefix_cp_2;

        /* 1. for data transform to save */
        rt_api_mask_apply(&prefix_cp_2, dest, prefixlen);

        /* 2. get route  */
        sal_memset(&static_rt, 0, sizeof(static_rt));
        static_rt.key.vrf_id = vrf_id;
        static_rt.key.p.family = dest->family;
        static_rt.key.p.prefixlen = prefixlen;
        static_rt.key.distance = distance;
        sal_memcpy (&static_rt.key.p, &prefix_cp_2, sizeof(prefix_t));

        /*ecmp*//*for full indirect routes, ecmp member change distance will add 1 indirect route, will exceed the max num, so here add check*/
        pst_static_rt = tbl_static_rt_cnt_get_static_rt_cnt(&static_rt.key);
        if (NULL == pst_static_rt)
        {
            if (AF_INET6 == dest->family)
            {
                if (rt_indirect_ipv6_route_count_max_check(1))
                {
                    //if (TRUE != rt_is_static_route_dest_exist(vrf_id, prefixlen, &prefix_cp))
                    {
                        if (!is_update_distance_tmp)
                        {
                            tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
                            logid_sys(LOG_ROUTE_6_INDIRECT_ROUTE_OVERFLOW, p_rt_glb->current_indirect_ipv6_route_num, 1, rt_fea_indirect_route_v6_count_get_max());
                            return PM_E_ROUTE_INDIRECT_ROUTE_FULL;
                            //CFG_CONFLICT_RET("ipv6 exceed max %u", rt_fea_indirect_route_v6_count_get_max());
                        }
                    }
                }
            }
            else
            {
                if (rt_indirect_ipv4_route_count_max_check(1))
                {
                    //if (TRUE != rt_is_static_route_dest_exist(vrf_id, prefixlen, &prefix_cp))
                    {
                        if (!is_update_distance_tmp)
                        {
                            return PM_E_ROUTE_INDIRECT_ROUTE_FULL;
                        }
                    }
                }
            }
        }

        /*distance is one member of tbl_static_rt_cnt key, so change distance should add and delete tbl_static_rt_cnt to update */
        rc = rt_cmd_static_rt_cnt_add(vrf_id, dest, 
            prefixlen, distance, dest->family == AF_INET ? TRUE : FALSE, gate, is_blackhole, is_update_distance_tmp, pst_static_route->distance, &count_add_failed);
        if (PM_E_NONE != rc)
        {
            return rc;
        }
        
        /*when resource is full, update distance, should delete, then add, otherwise will return PM_E_ROUTE_INDIRECT_ROUTE_FULL*/
        rt_cmd_static_rt_cnt_del(vrf_id, dest, prefixlen, pst_static_route->distance, gate->family == AF_INET ? TRUE : FALSE, gate, count_add_failed);
        
        pst_static_route->distance = distance;
        rc = tbl_static_route_cfg_set_static_route_cfg_field(pst_static_route, TBL_STATIC_ROUTE_CFG_FLD_DISTANCE);
        *is_update_distance = TRUE;
    }
    else if (NULL == pst_static_route)
    {
        rc = rt_cmd_static_rt_cnt_add(vrf_id, dest, prefixlen, distance, dest->family == AF_INET ? TRUE : FALSE, gate, is_blackhole, is_update_distance_tmp, 0, &count_add_failed);
        if (PM_E_NONE != rc)
        {
            return rc;
        }
        rc = tbl_static_route_cfg_add_static_route_cfg(&static_route);
        
    }
    else
    {
        *is_already_exist = TRUE;
    }

    if (track_id)
    {
        rc = track_ipv4_route_set(&static_route.key, track_id, is_add, para);  
    }

    return rc;
}

int32
rt_delete_ipv6_static_route_without_ifindex(addr_ipv6_t *dest, addr_ipv6_t *gate, uint8 prefixlen)
{
    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;
    int32 rc = PM_E_NONE;
    uint32 delete_count = 0;
    uint32 ifindex = 0;
    uint32 distance = 0;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_rst_cfg, listnode, next)
    {
        if (AF_INET == p_db_rst_cfg->key.p.family)
        {
            continue;
        }
        else
        {
            if (0 == sal_memcmp(&p_db_rst_cfg->key.p.u.prefix6, dest, sizeof(addr_ipv6_t)) 
                && p_db_rst_cfg->key.p.prefixlen == prefixlen 
                && 0 == sal_memcmp(&p_db_rst_cfg->key.nh_addr.ipv6, gate, sizeof(addr_ipv6_t)))
            {
                ifindex = p_db_rst_cfg->key.ifindex_v6;
                distance = p_db_rst_cfg->distance;
                
                rc = tbl_static_route_cfg_del_static_route_cfg(&p_db_rst_cfg->key);
                if (rc != PM_E_NONE)
                {
                    MY_PRINTF("delete failed");
                }

                prefix_t gate_prefix;
                sal_memset(&gate_prefix, 0, sizeof(gate_prefix));
                gate_prefix.family = AF_INET6;
                sal_memcpy(&gate_prefix.u.prefix6, gate, sizeof(gate_prefix.u.prefix6));

                prefix_t dest_prefix;
                sal_memset(&dest_prefix, 0, sizeof(dest_prefix));
                dest_prefix.family = AF_INET6;
                dest_prefix.prefixlen = prefixlen;
                sal_memcpy(&dest_prefix.u.prefix6, dest, sizeof(dest_prefix.u.prefix6));
                
                rt_cmd_static_rt_cnt_del(0, &dest_prefix, prefixlen, distance, FALSE, &gate_prefix, FALSE);
                rt_cmd_del_static_route_by_syscmd(&dest_prefix, &gate_prefix, prefixlen, ifindex, FALSE);

                /*not return for 
                    ipv6 route 2003::/64 fe80::1 eth-0-2 and 
                    ipv6 route 2003::/64 fe80::1 eth-0-1 then 
                    no ipv6 route 2003::2/64 fe80::1 
                    could delete all*/

                delete_count++;
            }
        }
    }

    if (delete_count == 0)
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int32
rt_cmd_remove_static_route(vrf_id_t vrf_id, char **argv, int32 argc, cfg_cmd_para_t *para, prefix_t *dest, 
    prefix_t *gate, uint8 prefixlen, uint32 ifindex, uint32 is_blackhole)
{
    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;
    prefix_t prefix_cp;
    //uint32      mask = 0;
    int32       rc = PM_E_NONE;
    uint32 distance = 0;
    uint32 dhcp_client = 0;

    if (NULL == dest || NULL == gate)
    {
        return PM_E_FAIL;
    }

    /* modified by liwh for bug 50196, 2019-01-07 */
    if ((argc >=7) && (!sal_strcmp(argv[5], "dhcp_client")))
    {
        PM_CFG_GET_UINT(dhcp_client, argv[6], para->p_rs);    
    }
    /* liwh end */

    /* 1. for data transform to save */
    rt_api_mask_apply(&prefix_cp, dest, prefixlen);

    /* 1. get route  */
    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = gate->family;
    static_route.key.p.prefixlen = prefixlen;
    if (AF_INET == gate->family)
    {
        sal_memcpy(&static_route.key.nh_addr.ipv4, &gate->u.prefix4, sizeof(addr_ipv4_t));
    }
    else
    {
        sal_memcpy(&static_route.key.nh_addr.ipv6, &gate->u.prefix6, sizeof(addr_ipv6_t));
        static_route.key.ifindex_v6 = ifindex;
    }
    sal_memcpy (&static_route.key.p, &prefix_cp, sizeof(prefix_t));

    pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);
    if (NULL != pst_static_route)
    {
        /* modified by liwh for bug 50196, 2019-01-07 
             when dhcp_client differs, shouldn't delete static route */
        if (dhcp_client != pst_static_route->dhcp_client)
        {
            return PM_E_NONE;
        }
        /* liwh end */
        
        distance = pst_static_route->distance;
        
        /* process cancel track static route first */
        /*add family check for no ipv6 route*/
        /* modified by liwh for bug 50196, 2019-01-07 */
        if (argc > 5 && AF_INET == dest->family && (0 == dhcp_client)) 
        //if (argc > 5 && AF_INET == dest->family) 
        /* liwh end */
        {
            track_ipv4_route_unset(&static_route.key, para);
            rc = rt_cmd_add_static_route_by_syscmd(dest, gate, prefixlen, pst_static_route->distance, ifindex, is_blackhole);
            return rc;
        }

        rc = tbl_static_route_cfg_del_static_route_cfg(&static_route.key);
        
        rt_cmd_static_rt_cnt_del(vrf_id, dest, prefixlen, distance, gate->family == AF_INET ? TRUE : FALSE, gate, FALSE);
    }
    else
    {
        /*for  ipv6 route 2003::1/64 fe80::1 eth-0-1 then no ipv6 route 2003::2/64 fe80::1 without interface*/
        /*for bug 49366 by chenc*/
        if (AF_INET6 == gate->family && ifindex == 0)
        {
            rc = rt_delete_ipv6_static_route_without_ifindex(&static_route.key.p.u.prefix6, &static_route.key.nh_addr.ipv6, static_route.key.p.prefixlen);
            if (rc == PM_E_NONE)
            {
                return rc;
            }
            /*delete not exist route will not print error*/
            //CFG_INVALID_PARAM_RET ("No matching route to delete");
            return PM_E_NONE;
        }
        else
        {
            return PM_E_NONE;
        }
    }

    rc = rt_cmd_del_static_route_by_syscmd(dest, gate, prefixlen, ifindex, is_blackhole);

    return rc;
}

/*/ip/${ip}/ipgw/${ipgw}/track_id*/
/*/ipv6/${ipv6}/ipv6_gw/${ipv6_gw}/interface/${iface_name}*/
int32
rt_cmd_remove_del_static_route_without_gw(vrf_id_t vrf_id, char **argv, int32 argc, cfg_cmd_para_t *para, prefix_t *dest, uint8 prefixlen)
{
    tbl_static_route_cfg_t  *pst_static_route = NULL;
    tbl_static_route_cfg_t  static_route;
    prefix_t prefix_cp;
//    uint32      mask = 0;
    int32 rc = PM_E_NONE;
    char buf4[CMD_BUF_64] = {'\0'};
    char buf3[CMD_BUF_64] = {'\0'};
    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;
    prefix_t gate_tmp = {0};
    uint32 ifindex = 0;
    uint32 del_count = 0;
    uint32 distance = 0;
    uint32 is_blackhole = FALSE;
    addr_ipv4_t     tmp_zero;
    addr_ipv6_t     tmp_zero1;

    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));
    sal_memset(&tmp_zero1, 0, sizeof(tmp_zero1));

    if (NULL == p_master || NULL == dest)
    {
        return PM_E_NONE;
    }

    /* 1. for data transform to save */
    rt_api_mask_apply(&prefix_cp, dest, prefixlen);

    /* 1. get route  */
    sal_memset(&static_route, 0, sizeof(static_route));
    static_route.key.vrf_id = vrf_id;
    static_route.key.p.family = prefix_cp.family;
    static_route.key.p.prefixlen = prefixlen;
    //sal_memcpy(&static_route.key.nh_addr.ipv4, gate, sizeof(addr_ipv4_t));
    sal_memcpy (&static_route.key.p, &prefix_cp, sizeof(prefix_t));
    
    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_rst_cfg, listnode, next)
    {
        if (NULL == p_db_rst_cfg)
        {
            continue;
        }

        is_blackhole = FALSE;

        sal_memset(buf3, 0, sizeof(buf3));
        if (AF_INET == prefix_cp.family)
        {
            cdb_addr_ipv4_val2str(buf3, CMD_BUF_64, &(p_db_rst_cfg->key.nh_addr.ipv4));
            if (0 == sal_memcmp(&tmp_zero, &(p_db_rst_cfg->key.nh_addr.ipv4), sizeof(tmp_zero)))
            {
                is_blackhole = TRUE;
            }
        }
        else
        {
            if (0 == sal_memcmp(&tmp_zero1, &(p_db_rst_cfg->key.nh_addr.ipv6), sizeof(tmp_zero1)))
            {
                is_blackhole = TRUE;
            }
            cdb_addr_ipv6_val2str(buf3, CMD_BUF_64, &(p_db_rst_cfg->key.nh_addr.ipv6));
        }
        
        sal_memset(buf4, 0, sizeof(buf4));
        cdb_prefix_val2str(buf4, CMD_BUF_64, &(p_db_rst_cfg->key.p));

        if (static_route.key.vrf_id == p_db_rst_cfg->key.vrf_id 
            && static_route.key.p.family == p_db_rst_cfg->key.p.family 
            && static_route.key.p.prefixlen == p_db_rst_cfg->key.p.prefixlen 
            && !sal_memcmp(&static_route.key.p, &p_db_rst_cfg->key.p, sizeof(prefix_t)))
        {
            /*add ipv6 static with if, but delete without gw and if, tbl_static_route_cfg and frr cannot delete,
            so add ifindex*/
            ifindex = p_db_rst_cfg->key.ifindex_v6;
            static_route.key.ifindex_v6 = ifindex;
            sal_memcpy(&static_route.key.nh_addr, &p_db_rst_cfg->key.nh_addr, sizeof(nexthop_key_u));
            pst_static_route = tbl_static_route_cfg_get_static_route_cfg(&static_route.key);

            /* process cancel track static route first */
            /*add family check for no ipv6 route*/
            if (argc > 5 && (AF_INET == dest->family))
            {
                sal_memset(&gate_tmp, 0, sizeof(gate_tmp));
                gate_tmp.family = dest->family;
                sal_memcpy(&gate_tmp.u.prefix4, &pst_static_route->key.nh_addr.ipv4, sizeof(addr_ipv4_t));
                sal_memcpy(&gate_tmp.u.prefix6, &pst_static_route->key.nh_addr.ipv6, sizeof(addr_ipv6_t));
                
                track_ipv4_route_unset(&static_route.key, para);
                // for delete ipv4 static route without gw, ifindex is 0
                rc = rt_cmd_add_static_route_by_syscmd(dest, &gate_tmp, prefixlen, pst_static_route->distance, 0, is_blackhole);
                continue;
            }

            if (NULL != pst_static_route)
            {
                prefix_t gate_prefix;
                sal_memset(&gate_prefix, 0, sizeof(gate_prefix));
                gate_prefix.family = (AF_INET == dest->family ? AF_INET : AF_INET6);
                sal_memcpy(&gate_prefix.u.prefix4, &static_route.key.nh_addr.ipv4, sizeof(gate_prefix.u.prefix4));
                sal_memcpy(&gate_prefix.u.prefix6, &static_route.key.nh_addr.ipv6, sizeof(gate_prefix.u.prefix6));
                
                /*delete tbl*/
                distance = pst_static_route->distance;
                rc = tbl_static_route_cfg_del_static_route_cfg(&static_route.key);
                rt_cmd_static_rt_cnt_del(vrf_id, dest, prefixlen, distance, dest->family== AF_INET ? TRUE : FALSE, &gate_prefix, FALSE);
            }

            sal_memset(&gate_tmp, 0, sizeof(gate_tmp));
            gate_tmp.family = dest->family;
            sal_memcpy(&gate_tmp.u.prefix4, &static_route.key.nh_addr.ipv4, sizeof(addr_ipv4_t));
            sal_memcpy(&gate_tmp.u.prefix6, &static_route.key.nh_addr.ipv6, sizeof(addr_ipv6_t));

            // for delete ipv6, add ifindex
            rc = rt_cmd_del_static_route_by_syscmd(dest, &gate_tmp, prefixlen, ifindex, is_blackhole);

            del_count++;
        }
    }

    if (0 == del_count && AF_INET6 == dest->family)
    {
        /*delete not exist route will not print error*/
        //CFG_INVALID_PARAM_RET ("No matching route to delete");
    }

    return rc;
}

int32
rt_cmd_process_static_rt_cnt(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    int32           rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_STATIC_RT_CNT);

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_static_rt_cnt(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

/* modified by liul for bug 21047, 2012-11-08. Filter IPv4 Embeded IPv6 address */
#define IN6_IS_ADDR_V4_EMBEDDED_V6(a)               \
        (((a)->s6_addr32[0] & htonl(0xE0000000)) == htonl(0x00000000) )
        
int32
rt_cmd_check_prefix_for_ipv6_route_add(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    new_cmd_args_key_value_t key_value;
    int32 key_num = 1;
    prefix_t prefix = {0};
    int32 rc = 0;

    sal_memset(&prefix, 0, sizeof (prefix));
    CFG_PARSER_PREFIX(argv[1], argv[2], &prefix, para->p_rs);
    
    if (IPV6_PREFIX_IS_LINK_LOCAL(&prefix))
    {
        CFG_INVALID_PARAM_RET ("Destination address can't be link-local");
    }
    else if (IPV6_PREFIX_IS_MULTICAST(&prefix))
    {
        CFG_INVALID_PARAM_RET ("Destination address can't be multicast");
    }
    else if (IPV6_PREFIX_IS_LOOPBACK(&prefix))
    {
        CFG_INVALID_PARAM_RET ("Destination address can't be loopback");
    }
    else if (IPV6_PREFIX_IS_UNSPECIFIED(&prefix))
    {
        CFG_INVALID_PARAM_RET ("Destination address can't be unspecified");
    }
    else if (IPV6_PREFIX_IS_LOOPBACK_AND_UNSPECIFIED(&prefix))
    {
        CFG_INVALID_PARAM_RET ("Destination address can't be loopback and unspecified");
    }
    else if (IN6_IS_ADDR_LINKLOCAL(&prefix.u.prefix6) && prefix.prefixlen < 10)
    {
        CFG_INVALID_PARAM_RET ("Invaild destination address");
    }
    /*fix bug 49378*/
    else if (IN6_IS_ADDR_V4MAPPED(&prefix.u.prefix6))
    {
        CFG_INVALID_PARAM_RET ("Destination address can't be IPv4-mapped IPv6 address");
    }

    addr_ipv6_t gate;
    sal_memset(&gate, 0, sizeof (gate));
      /*fix bug 53927,for nexthop ipv6 == null ,by guoxd*/
    if( 0 != sal_strcmp (argv[3], "null"))
    {
        CFG_PARSER_IPV6_ADDR(argv[4], &gate, para->p_rs);
    }
    sal_memset(&key_value, 0, sizeof(key_value));
    sal_strcpy(key_value.key_name, "interface");
    cfg_parser_cmd_args_new(argv, argc, para, &key_value, key_num, FALSE);
    /*get interface*//*configure with interface*/
    if (0 != sal_strlen(key_value.key_value))
    {
        if (IN6_IS_ADDR_MULTICAST (&gate)
            || IN6_IS_ADDR_LOOPBACK (&gate)
            || IN6_IS_ADDR_UNSPECIFIED (&gate))
        {
            CFG_INVALID_PARAM_RET ("Invalid nexthop address");
        }

        tbl_interface_t *p_db_if = tbl_interface_get_interface_by_name(key_value.key_value);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET ("Invalid interface %s", key_value.key_value);
        }
        else
        {
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                CFG_CONFLICT_RET("Cannot configure LAG member port as output interface");
            }
        
            if (p_db_if->mode != GLB_IF_MODE_L3)
            {
                CFG_INVALID_PARAM_RET ("IPv6 commands not allowed on L2 interface");
            }

            if (!IN6_IS_ADDR_LINKLOCAL (&gate))
            {
                CFG_INVALID_PARAM_RET ("Interface only required for link-local nexthop address");
            }
        }
    }
    else
    /*configure without interface*/
    {
        /*fix bug 53927,for nexthop ipv6 == null ,by guoxd*/
        if( 0 == sal_strcmp (argv[3], "null"))
        {
            return PM_E_NONE;
        }
        if (IN6_IS_ADDR_MULTICAST (&gate)
	   || IN6_IS_ADDR_LOOPBACK (&gate)
	   || IN6_IS_ADDR_UNSPECIFIED (&gate))
	   /*fix bug 48532 by chenc*/
	   //|| IN6_IS_ADDR_V4_EMBEDDED_V6((&gate)))
        {
            CFG_INVALID_PARAM_RET ("Invalid nexthop address");
        }

        /*fix bug 49378*/
        if (IN6_IS_ADDR_V4MAPPED (&gate))
        {
            CFG_INVALID_PARAM_RET ("Invalid nexthop address");
        }
        
        if (IN6_IS_ADDR_LINKLOCAL (&gate))
        {
            CFG_INVALID_PARAM_RET ("Interface has to be specified for a link-local nexthop");
        }
    }

    return PM_E_NONE;
}

int32
rt_cmd_check_prefix_for_ipv6_route_del(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    new_cmd_args_key_value_t key_value;
    int32 key_num = 1;
    int32 rc = 0;

    addr_ipv6_t gate;
    sal_memset(&gate, 0, sizeof (gate));
    /*get nexthop*/
    /*configure without nexthop, argv[4] is \0*/
    if (0 != sal_strcmp(argv[3], "null") && 0 == sal_strlen(argv[4]))
    {
        return PM_E_NONE;
    }
    else
    {
        /*fix bug 53927,for nexthop ipv6 == null ,by guoxd*/
        if(0 != sal_strcmp(argv[3], "null"))
        {
            CFG_PARSER_IPV6_ADDR(argv[4], &gate, para->p_rs);
        }
    }

    sal_memset(&key_value, 0, sizeof(key_value));
    sal_strcpy(key_value.key_name, "interface");
    cfg_parser_cmd_args_new(argv, argc, para, &key_value, key_num, FALSE);
    /*get interface*/
    /*configure with interface*/
    if (0 != sal_strlen(key_value.key_value))
    {
        if (IN6_IS_ADDR_MULTICAST (&gate)
            || IN6_IS_ADDR_LOOPBACK (&gate)
            || IN6_IS_ADDR_UNSPECIFIED (&gate))
        {
            CFG_INVALID_PARAM_RET ("Invalid nexthop address");
        }

        tbl_interface_t *p_db_if = tbl_interface_get_interface_by_name(key_value.key_value);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET ("Invalid interface %s", key_value.key_value);
        }
        else
        {
            if (INTERFACE_IS_LAG_MEMBER(p_db_if))
            {
                CFG_CONFLICT_RET("Cannot configure LAG member port as output interface");
            }
        
            if (p_db_if->mode != GLB_IF_MODE_L3)
            {
                CFG_INVALID_PARAM_RET ("IPv6 commands not allowed on L2 interface");
            }

            if (!IN6_IS_ADDR_LINKLOCAL (&gate))
            {
                CFG_INVALID_PARAM_RET ("Interface only required for link-local nexthop address");
            }
        }
    }
    else
    /*configure without interface*/
    {
    /*fix bug 53927,for nexthop ipv6 == null ,by guoxd*/
        if( 0 == sal_strcmp (argv[3] , "null") )
        {
            return PM_E_NONE;
        }
        
        if (IN6_IS_ADDR_MULTICAST (&gate)
	   || IN6_IS_ADDR_LOOPBACK (&gate)
	   || IN6_IS_ADDR_UNSPECIFIED (&gate))
	   /*fix bug 49448 by chenc*/
	   //|| IN6_IS_ADDR_V4_EMBEDDED_V6((&gate)))
        {
            CFG_INVALID_PARAM_RET ("Invalid nexthop address");
        }

        /*fix bug 49378*/
        if (IN6_IS_ADDR_V4MAPPED (&gate))
        {
            CFG_INVALID_PARAM_RET ("Invalid nexthop address");
        }
    }

    return PM_E_NONE;
}

int32 
rt_cmd_check_whether_arp_nd_duplicate_with_dest(prefix_t *dest, prefix_t *gate,
                        uint8 prefixlen, uint32 distance, uint32 ifindex)
{
    tbl_arp_t *p_db_arp = NULL;
    tbl_arp_key_t arp_key;
    char buf1[CMD_BUF_64] = {0}; 
    char buf2[CMD_BUF_64] = {0}; 
    char buf3[CMD_BUF_64] = {0}; 
    prefix_t prefix_cp;
    ds_static_route_dup_ipv4_nh_t ds_static_route_dup_ipv4_nh;
    ds_static_route_dup_ipv4_nh_t *p_db_ds_static_route_dup_ipv4_nh = NULL;

    tbl_nd_fib_t *p_db_nd_fib = NULL;
    tbl_nd_fib_key_t nd_fib_key;

    rt_api_mask_apply(&prefix_cp, dest, prefixlen);

    /*ipv4*/
    if (AF_INET6 != dest->family)
    {
        sal_memset(&arp_key, 0x00, sizeof(arp_key));
        sal_memcpy(&arp_key.ip, &prefix_cp.u.prefix4, sizeof(addr_ipv4_t));
        p_db_arp = tbl_arp_get_arp(&arp_key);
        if (p_db_arp && GLB_FLAG_ISSET(p_db_arp->flags, ARP_SYN_HW))
        {
            p_db_arp->is_static_route_dup = TRUE;
            sal_memset(&ds_static_route_dup_ipv4_nh, 0x00, sizeof(ds_static_route_dup_ipv4_nh));
            sal_memcpy(&ds_static_route_dup_ipv4_nh.key.address, &gate->u.prefix4, sizeof(addr_ipv4_t));
            p_db_ds_static_route_dup_ipv4_nh = ds_static_route_dup_ipv4_nh_get_static_route_dup_ipv4_nh(p_db_arp, &ds_static_route_dup_ipv4_nh);
            if (p_db_ds_static_route_dup_ipv4_nh)
            {
                
            }
            else
            {
                ds_static_route_dup_ipv4_nh_add_static_route_dup_ipv4_nh(p_db_arp, &ds_static_route_dup_ipv4_nh);
                tbl_arp_set_arp_field(p_db_arp, TBL_ARP_FLD_IS_STATIC_ROUTE_DUP);
            }

            log_sys(M_MOD_ROUTE, E_INFORMATIONAL, "Not add static route %s/%d %s to rib for arp %s exist\n", 
                cdb_addr_ipv4_val2str(buf1, CMD_BUF_64, &prefix_cp.u.prefix4),
                prefixlen, 
                cdb_addr_ipv4_val2str(buf2, CMD_BUF_64, &gate->u.prefix4),
                cdb_addr_ipv4_val2str(buf3, CMD_BUF_64, &p_db_arp->key.ip));
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        sal_memset(&nd_fib_key, 0x00, sizeof(nd_fib_key));
        /*not link-local, ifindex is 0*/
        nd_fib_key.ifindex = 0;
        nd_fib_key.addr.family = AF_INET6;
        nd_fib_key.addr.prefixlen = 128;
        sal_memcpy(&nd_fib_key.addr.u.prefix6, &prefix_cp.u.prefix6, sizeof(addr_ipv6_t));
        p_db_nd_fib = tbl_nd_fib_get_nd_fib(&nd_fib_key);
        if (p_db_nd_fib && GLB_FLAG_ISSET(p_db_nd_fib->flags, GLB_ND_FLAG_ACTIVE))
        {
            p_db_nd_fib->is_static_route_dup = TRUE;
            sal_memcpy(&p_db_nd_fib->staic_route_dup_ipv6_nh, &gate->u.prefix6, sizeof(addr_ipv6_t));
            tbl_nd_fib_set_nd_fib_field(p_db_nd_fib, TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP);
            tbl_nd_fib_set_nd_fib_field(p_db_nd_fib, TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH);

            log_sys(M_MOD_ROUTE, E_INFORMATIONAL, "Not add static route %s/%d %s to rib for nd %s exist\n", 
                cdb_addr_ipv6_val2str(buf1, CMD_BUF_64, &prefix_cp.u.prefix6),
                prefixlen, 
                cdb_addr_ipv6_val2str(buf2, CMD_BUF_64, &gate->u.prefix6),
                cdb_addr_ipv6_val2str(buf3, CMD_BUF_64, &p_db_nd_fib->key.addr.u.prefix6));
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

/*  0    1      2          3           4               5             6                  7*/
/*ip/${ip}/ipgw/${ipgw}/distance/${distance}/track_id/${track_id}*/
/*ip/${ip}/ipgw/${ipgw}*/

/*add*/
/*ip/${ip}/ipgw/${ipgw}/distance/${distance}/track_id/${track_id}*/
/*ip/${ip}/null/${null_value}/distance/${distance}/track_id/${track_id}*/

/*del*/
/*ip/${ip}/ipgw/${ipgw}/track_id*/
/*ip/${ip}/ipgw/${ipgw}*/
/*ip/${ip}/null/${null_value}*/


/*  0          1          2              3               4               5             6                  7*/
/*ipv6/${ipv6}/ipv6_gw/${ipv6_gw}/distance/${distance}/interface/${iface_name}*//*add*/
/*ipv6/${ipv6}/ipv6_gw/${ipv6_gw}/interface/${iface_name}*//*del*/
int32
rt_cmd_process_static_route_cfg(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node = NULL;
    field_parse_t   field;
    vrf_id_t        vrf_id = 0;
    prefix_t     dest;
    prefix_t     gate;
    addr_ipv4_t     tmp_zero;
    uint8           prefixlen = 0;
    uint32          distance = 0;
    uint16          track_id = 0;
    uint16          is_add = 1;
    uint32          is_already_exist = FALSE;
    int32           rc = PM_E_NONE;
    uint32 is_ipv4 = TRUE;
    uint32 ifindex = 0;
    int32 is_update_distance = FALSE;
    uint32 is_blackhole = FALSE;
    uint32 dhcp_client = 0;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_STATIC_ROUTE_CFG);

    sal_memset(&dest, 0, sizeof(dest));
    sal_memset(&gate, 0, sizeof(gate));
    sal_memset(&tmp_zero, 0, sizeof(tmp_zero));

    _cmd_print_para(argv, argc);
    
    switch (para->oper)
    {
    /*cmd : ip route */
    case CDB_OPER_ADD:
        if (0 == sal_strcmp(argv[0], "ip"))
        {
            gate.family = AF_INET;
            dest.family = AF_INET;
            is_ipv4 = TRUE;
        }
        else if (0 == sal_strcmp(argv[0], "ipv6"))
        {
            if (FALSE == rt_api_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }
    
            gate.family = AF_INET6;
            dest.family = AF_INET6;
            is_ipv4 = FALSE;
            rc = rt_cmd_check_prefix_for_ipv6_route_add(argv, argc, para);
            if (rc != PM_E_NONE)
            {
                return rc;
            }
        }
        else
        {
            MY_PRINTF("err static route cfg");
        }

        if (!sal_strcmp(argv[3], "null"))
        {
            is_blackhole = TRUE;
        }

        if (!sal_strcmp(argv[7], "dhcp_client"))
        {
            PM_CFG_GET_UINT(dhcp_client, argv[8], para->p_rs);    
        }
    
        rc = rt_cmd_static_route_data_check(argv, argc, para);
        if (PM_E_CFG_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("Please check route CLI configuration!");
        }

        if (PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The member of ecmp is more than maximum %d!", 
                is_ipv4 ? rt_fea_ecmp_member_count_get_max() : rt_fea_v6_ecmp_member_count_get_max());
            return PM_E_NONE;
        }

        rc = rt_cmd_static_route_data_transform(argv, argc, para, &vrf_id, &dest, &gate,
                            &prefixlen, &distance, &track_id, &ifindex, is_blackhole);
        if (rc != 0)
        {
            return rc;
        }
        
        /*fix bug 44611, when interface is down, it can limit the ecmp member number*/
        rc = rt_is_static_rt_ecmp_member_exceed(vrf_id, &dest, &gate, prefixlen, distance);
        if (TRUE == rc)
        {
            CFG_INVALID_PARAM_RET("The member of ecmp is more than maximum %d!", 
                is_ipv4 ? rt_fea_ecmp_member_count_get_max() : rt_fea_v6_ecmp_member_count_get_max());
        }
        
        rc = rt_cmd_create_static_route(vrf_id, para, &dest, &gate, prefixlen, distance, track_id, ifindex, &is_add, &is_update_distance, &is_already_exist, is_blackhole, dhcp_client);
        if (PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The static route entry num is more than maximum %d!", 
                is_ipv4 ? rt_fea_static_route_count_get_max() : rt_fea_static_route_v6_count_get_max());
        }

        if (PM_E_ECMP_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The ecmp entry num is more than maximum %d!", 
                is_ipv4 ? rt_fea_ecmp_count_get_max() : rt_fea_v6_ecmp_count_get_max());
        }

        if (PM_E_ROUTE_DEST_SAME_WITH_NEXTHOP == rc)
        {
            CFG_INVALID_PARAM_RET("Nexthop cannot same with destination address !");
        }

        if (PM_E_ROUTE_INDIRECT_ROUTE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("IPv%s indirect route entry num is more than maximum %u", is_ipv4 ? "4" : "6", is_ipv4 ? rt_fea_indirect_route_count_get_max(): rt_fea_indirect_route_v6_count_get_max());
        }

        if (PM_E_ROUTE_HOST_ROUTE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("IPv%s host route entry num is more than maximum %u", is_ipv4 ? "4" : "6", is_ipv4 ? arp_api_count_get_max(): nd_api_count_get_max());
        }

        if (is_add)
        {
            /*for bug 48527 by chenc*/
            if (is_update_distance)
            {
                rc = rt_cmd_del_static_route_by_syscmd(&dest, &gate, prefixlen, ifindex, is_blackhole);
                usleep(10000);
            }

            if (!is_already_exist)
            {
                if (!is_blackhole)
                {
                    /*check whether arp or nd is duplicate with dest*/
                    rc = rt_cmd_check_whether_arp_nd_duplicate_with_dest(&dest, &gate, prefixlen, distance, ifindex);
                    if (TRUE == rc)
                    {
                        return PM_E_NONE;
                    }
                }
                
                rc = rt_cmd_add_static_route_by_syscmd(&dest, &gate, prefixlen, distance, ifindex, is_blackhole);
            }
        }
        else
        {
            rc = rt_cmd_del_static_route_by_syscmd(&dest, &gate, prefixlen, ifindex, is_blackhole);
        }

        break;

    /*cmd : no ip route*/
    case CDB_OPER_DEL:
        if (0 == sal_strcmp(argv[0], "ip"))
        {
            gate.family = AF_INET;
            dest.family = AF_INET;
            is_ipv4 = TRUE;
        }
        else if (0 == sal_strcmp(argv[0], "ipv6"))
        {
            if (FALSE == rt_api_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }
            
            gate.family = AF_INET6;
            dest.family = AF_INET6;
            is_ipv4 = FALSE;
            
            rc = rt_cmd_check_prefix_for_ipv6_route_del(argv, argc, para);
            if (rc != PM_E_NONE)
            {
                return rc;
            }
        }
        else
        {
            MY_PRINTF("err static route cfg");
        }

        if (!sal_strcmp(argv[3], "null"))
        {
            is_blackhole = TRUE;
        }
        
        rc = rt_cmd_static_route_data_transform(argv, argc, para, &vrf_id, &dest, &gate,
                            &prefixlen, &distance, &track_id, &ifindex, is_blackhole);
        if (is_ipv4)
        {
            if (0 != sal_memcmp(&tmp_zero, &gate.u.prefix4, sizeof(tmp_zero)))
            {
                rc = rt_cmd_remove_static_route(vrf_id, argv, argc, para, &dest, &gate, prefixlen, ifindex, is_blackhole);
            }
            else
            {
                if (is_blackhole)
                {
                    rc = rt_cmd_remove_static_route(vrf_id, argv, argc, para, &dest, &gate, prefixlen, ifindex, is_blackhole);
                }
                else
                {
                    /*gateway is not given*/
                    rc = rt_cmd_remove_del_static_route_without_gw(vrf_id, argv, argc, para, &dest, prefixlen);
                }
            }
        }
        else
        {
            /*ipv6/${ipv6}/ipv6_gw/${ipv6_gw}/interface/${iface_name}*/
            if ((argc == 6) && (0 == sal_strlen(argv[4])))
            {
                /*del all*/
                /* no ipv6 route 2001::1/24*/
                rc = rt_cmd_remove_del_static_route_without_gw(vrf_id, argv, argc, para, &dest, prefixlen);
            }
            else if ((argc == 6) && (0 != sal_strlen(argv[4])))
            {
                /* no ipv6 route 2001::1/24 1::1*/
                /*ifindex is 0*/
                rc = rt_cmd_remove_static_route(vrf_id, argv, argc, para, &dest, &gate, prefixlen, 0, is_blackhole);
            }
            else if ((argc == 7) && (0 == sal_strcmp(argv[0], "ipv6")) && (0 != sal_strlen(argv[4])) && (0 != sal_strlen(argv[6])))
            {
                /* no ipv6 route 2001::1/24 1::1*/
                rc = rt_cmd_remove_static_route(vrf_id, argv, argc, para, &dest, &gate, prefixlen, ifindex, is_blackhole);
            }
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_static_route_cfg(argv[0], para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

/* modified by liwh for bug 51014, 2019-03-28 */
void
rt_ospf_redistribute_connected_timer(void *p_data)
{
    tbl_ospf_t  *p_db_ospf = NULL;

    if (!p_data)
    {
        return;    
    }
     
    p_db_ospf = (tbl_ospf_t*)p_data;
    p_db_ospf->t_ospf_redistribute_connected_timer = NULL;

    return;
}

void
rt_ospf_redistribute_static_timer(void *p_data)
{
    tbl_ospf_t  *p_db_ospf = NULL;

    if (!p_data)
    {
        return;    
    }
     
    p_db_ospf = (tbl_ospf_t*)p_data;
    p_db_ospf->t_ospf_redistribute_static_timer = NULL;

    return;
}

void rt_clear_ospf_timer(tbl_ospf_t  *p_db_ospf)
{
    if (!p_db_ospf)
    {
        return;    
    }
    
    if (GLB_IS_BIT_SET(p_db_ospf->redistribute, RT_ROUTE_CONNECT))
    {
        CTC_TASK_STOP_TIMER(p_db_ospf->t_ospf_redistribute_connected_timer);
    }

    if (GLB_IS_BIT_SET(p_db_ospf->redistribute, RT_ROUTE_STATIC))
    {
        CTC_TASK_STOP_TIMER(p_db_ospf->t_ospf_redistribute_static_timer);
    }

    return;
}
/* liwh end */

void rt_clear_ospf6_timer(tbl_ospf6_t  *p_db_ospf6)
{
    if (!p_db_ospf6)
    {
        return;    
    }
    
    if (GLB_IS_BIT_SET(p_db_ospf6->redistribute, RT_ROUTE_CONNECT))
    {
        CTC_TASK_STOP_TIMER(p_db_ospf6->t_ospf6_redistribute_connected_timer);
    }

    if (GLB_IS_BIT_SET(p_db_ospf6->redistribute, RT_ROUTE_STATIC))
    {
        CTC_TASK_STOP_TIMER(p_db_ospf6->t_ospf6_redistribute_static_timer);
    }

    return;
}


int32
rt_cmd_process_bgp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
	tbl_bgp_t						*p_db_bgp= NULL;
    tbl_bgp_t                      	bgp;
    tbl_bgp_key_t                  	bgp_key;
    //field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
			
   
	uint32		as_id = 0;
	addr_ipv4_t	router_id;
	uint32 		value;
    char        buf[64];

    cdb_field_t *p_field            = NULL;
    tbl_bgp_t  *p_bgp				= NULL;
	int32       source              = 0;
    int32       field_id            = 0;
    int32       retv = 0;

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	tbl_bgp_master_t 		*p_master = NULL;

    CFG_DBG_FUNC();

	
    p_master = tbl_bgp_get_master();
    if (NULL == p_master)
    {
        return rc;
    }
    //sal_memset(&field, 0, sizeof(field));
    sal_memset(&bgp, 0, sizeof(bgp));
    sal_memset(&router_id, 0, sizeof(router_id));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&bgp_key, 0, sizeof(bgp_key));
    p_node = cdb_get_tbl(TBL_BGP);

	switch (para->oper)
	{
	case CDB_OPER_ADD:
		if (2 == argc)
		{
            PM_CFG_GET_UINT(as_id, argv[1], para->p_rs);
            bgp_key.as_id = as_id;
            p_bgp = tbl_bgp_get_bgp(&bgp_key);
			if (NULL == p_bgp && 0 == sal_strcmp(argv[0], "create_bgp"))
			{
			    CTCLIB_SLIST_LOOP_DEL(p_master->bgp_list, p_bgp, listnode, next)
                {
                    if (p_bgp->key.as_id)
                    {
                        CFG_CONFLICT_RET("BGP is already running; AS is %u", p_bgp->key.as_id);
                    }                    
                }
				bgp.key.as_id = as_id;
				rc = tbl_bgp_add_bgp(&bgp);
                if (rc == 0)
                {
                    bgp_key_id = as_id;
                }
			}
		}
		break;	
		
	case CDB_OPER_DEL:
        if(0 == sal_strcmp(argv[0], "remove_bgp"))
        {
            _rt_cmd_del_bgp_process();
        }
        break;
		
   	case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        bgp_key.as_id = bgp_key_id;
        p_bgp = tbl_bgp_get_bgp(&bgp_key);
        if (NULL == p_bgp)
        {
            bgp.key.as_id = bgp_key_id;
			rc = tbl_bgp_add_bgp(&bgp);
        }
        p_db_bgp = tbl_bgp_get_bgp(&bgp_key);
        
        switch (field_id)
        {
         case TBL_BGP_FLD_ROUTER_ID:
			if ( 0 == sal_strcmp(argv[0], "router_id"))
        	{
        	    if( 0 == sal_strcmp(argv[1], "remove"))
                {
                    /*retv = sal_inet_pton(AF_INET, argv[2], buf);
                    if (!retv)
                    {
                        CFG_INVALID_PARAM_RET("Invalid router-id");
                    }*/
                    if (NULL != p_db_bgp)
                    {
                        //sal_memcpy(&router_id, buf, sizeof(addr_ipv4_t));
                        //if (0 == sal_memcmp(&p_db_bgp->router_id, &router_id, sizeof(addr_ipv4_t)))
                        {
                            sal_memset(&router_id, 0, sizeof(addr_ipv4_t));
                            sal_memcpy(&p_db_bgp->router_id, &router_id, sizeof(addr_ipv4_t));
                            tbl_bgp_set_bgp_field(p_db_bgp, TBL_BGP_FLD_ROUTER_ID);
                        }
                        /*else
                        {
                            CFG_INVALID_PARAM_RET("BGP router-id does not match");
                        }*/
                    }
                }
                else
                {
              		retv = sal_inet_pton(AF_INET, argv[1], buf);
                    if (!retv)
                    {
                        CFG_INVALID_PARAM_RET("Invalid router-id");
                    }
                   
                    if (NULL != p_db_bgp)
                    {
                        sal_memcpy(&router_id, buf, sizeof(addr_ipv4_t));
                        if (0 == router_id.s_addr)
                        {
                            CFG_INVALID_PARAM_RET("Invalid router-id");
                        }
                        if (0 != sal_memcmp(&p_db_bgp->router_id, &router_id, sizeof(addr_ipv4_t)))
                        {
                            sal_memcpy(&p_db_bgp->router_id, &router_id, sizeof(addr_ipv4_t));
                            tbl_bgp_set_bgp_field(p_db_bgp, TBL_BGP_FLD_ROUTER_ID);
                        }
                    }
                }
			}
			break;

		case TBL_BGP_FLD_REDISTRIBUTE:
			PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            if (0 == sal_strcmp(argv[1], "add"))
            {
                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                if (NULL != p_db_bgp)
                {
                    if (!GLB_IS_BIT_SET(p_db_bgp->redistribute, source))
                    {
                        GLB_SET_BIT(p_db_bgp->redistribute, source);
						tbl_bgp_set_bgp_field(p_db_bgp, TBL_BGP_FLD_REDISTRIBUTE);
                    }
					if (argc>3 && 0 == sal_strcmp(argv[3], "red_metric"))
					{
						PM_CFG_GET_UINT(value, argv[4], para->p_rs);
						p_db_bgp->red_metric[source] = value+1;//0-16 -> 1-17
						tbl_bgp_set_bgp_field(p_db_bgp, TBL_BGP_FLD_RED_METRIC);
					}
                }
            }

            if (0 == sal_strcmp(argv[1], "del"))
            {
                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                if (NULL != p_db_bgp)
                {
                    if (GLB_IS_BIT_SET(p_db_bgp->redistribute, source))
                    {
                        GLB_CLEAR_BIT(p_db_bgp->redistribute, source);
						p_db_bgp->red_metric[source] = 0;
                        tbl_bgp_set_bgp_field(p_db_bgp, TBL_BGP_FLD_REDISTRIBUTE);
                    }
                }
            }
            break;

		/*case TBL_BGP_FLD_DEFAULT_METRIC:
			if (2 == argc && 0 == sal_strcmp(argv[0], "default_metric"))
			{
				PM_CFG_GET_UINT(value, argv[1], para->p_rs);
				if (NULL != p_db_bgp)
				{
					if (p_db_bgp->default_metric != value)
					{
						p_db_bgp->default_metric = value;
						tbl_rip_set_rip_field(p_db_bgp, TBL_RIP_FLD_DEFAULT_METRIC);
					}
				}			
			}
			
			break;*/

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
rt_cmd_process_bgp_network(char **argv, int32 argc, cfg_cmd_para_t *para)
{    

    cdb_node_t *p_node              = NULL;
	tbl_bgp_network_t              	bgp_net;
	tbl_bgp_network_key_t			bgp_net_key;
	tbl_bgp_network_t				*p_db_bgp_network = NULL;



    int32       rc		= PM_E_NONE;
    int32       retv 	= 0;
    int32       field_id            = 0;
	addr_ipv4_t       mask;
	char        buf[64];
	addr_ipv4_t ip_prefix;

	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&bgp_net_key, 0, sizeof(bgp_net_key));
    sal_memset(&bgp_net, 0, sizeof(bgp_net));
    p_node = cdb_get_tbl(TBL_BGP_NETWORK);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_bgp_network_t 				*p_bgp_network = NULL;
	tbl_bgp_network_master_t 		*p_master = NULL;
    p_master = tbl_bgp_network_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (3 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }
            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));   
            
            sal_memset(buf, 0, sizeof(buf));
            retv = sal_inet_pton(AF_INET, argv[2], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid netmask prefix");
            }
            sal_memcpy(&mask, buf, sizeof(addr_ipv4_t));

		/*#ifdef _CENTEC_
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
		#endif*/
            sal_memcpy(&bgp_net_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            bgp_net_key.family = AF_INET;
            p_db_bgp_network = tbl_bgp_network_get_bgp_network(&bgp_net_key);
            if (NULL == p_db_bgp_network)
            {
                /*CTCLIB_SLIST_LOOP_DEL(p_master->bgp_network_list, p_bgp_network, listnode, next)
                {
                    if (neighbor_addr_match(&p_bgp_network->key, &bgp_net_key))
                    {
                        return rc;
                    }                    
                }*/

                sal_memcpy(&bgp_net.key, &bgp_net_key, sizeof(bgp_net_key));
                sal_memcpy(&bgp_net.mask, &mask, sizeof(addr_ipv4_t));
                tbl_bgp_network_add_bgp_network(&bgp_net);
            }
        }
        break;
	case CDB_OPER_DEL:
		if (3 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&bgp_net_key.u.prefix4, buf, sizeof(addr_ipv4_t));
            
            sal_memset(buf, 0, sizeof(buf));
            retv = sal_inet_pton(AF_INET, argv[2], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid netmask prefix");
            }
            sal_memcpy(&mask, buf, sizeof(addr_ipv4_t));
            /*
            #ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif*/
            bgp_net_key.family = AF_INET;
            p_db_bgp_network = tbl_bgp_network_get_bgp_network(&bgp_net_key);
            if (NULL != p_db_bgp_network)
            {
                if(sal_memcmp(&mask,&p_db_bgp_network->mask,sizeof(addr_ipv4_t)))
                {
                    CFG_INVALID_PARAM_RET("netmask unmatch");
                }
                tbl_bgp_network_del_bgp_network(&bgp_net_key);
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
rt_cmd_process_bgp_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para)
{    

    cdb_node_t *p_node              = NULL;
	tbl_bgp_neighbor_t             	bgp_nb;
	tbl_bgp_neighbor_key_t			bgp_nb_key;
	tbl_bgp_neighbor_t				*p_db_bgp_neighbor = NULL;
    cdb_field_t *p_field            = NULL;



    int32       rc		= PM_E_NONE;
    int32       retv 	= 0;
    uint32      remote_as = 0;
    uint32      ebgp_multihop = 0;
    uint32      shutdown = 0;
    uint32      next_hop_self = 0;
    int32       field_id            = 0;
	char        buf[64];
	char        buff[64];
	addr_ipv4_t ip_prefix;

	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
	sal_memset(buff, 0, sizeof(buff));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&bgp_nb_key, 0, sizeof(bgp_nb_key));
    sal_memset(&bgp_nb, 0, sizeof(bgp_nb));
    p_node = cdb_get_tbl(TBL_BGP_NEIGHBOR);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_bgp_neighbor_t 				*p_bgp_neighbor = NULL;
	tbl_bgp_neighbor_master_t 		*p_master = NULL;
    p_master = tbl_bgp_neighbor_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
        retv = sal_inet_pton(AF_INET, argv[0], buf);
        if (!retv)
        {
            CFG_INVALID_PARAM_RET("Invalid neighbor prefix");
        }

        sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
        PM_CFG_GET_UINT(remote_as, argv[2], para->p_rs);

		/*#ifdef _CENTEC_
        ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
		#endif*/

        bgp_nb_key.family = AF_INET;
        sal_memcpy(&bgp_nb_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
        p_db_bgp_neighbor = tbl_bgp_neighbor_get_bgp_neighbor(&bgp_nb_key);
        if (NULL == p_db_bgp_neighbor)
        {
            /*CTCLIB_SLIST_LOOP_DEL(p_master->bgp_neighbor_list, p_bgp_neighbor, listnode, next)
            {
                if (neighbor_addr_match((addr_t *)&p_bgp_neighbor->key, (addr_t *)&bgp_nb_key))
                {
                    return rc;
                }                    
            }
*/
            sal_memcpy(&bgp_nb.key, &bgp_nb_key, sizeof(bgp_nb_key));
            bgp_nb.remote_as = remote_as;
            tbl_bgp_neighbor_add_bgp_neighbor(&bgp_nb);
        }
        else
        {
            p_db_bgp_neighbor->remote_as = remote_as;
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_REMOTE_AS);
        }
        
        break;
	case CDB_OPER_SET:
        retv = sal_inet_pton(AF_INET, argv[0], buf);
        if (!retv)
        {
            CFG_INVALID_PARAM_RET("Invalid neighbor prefix");
        }

        sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));

        /*#ifdef _CENTEC_
        ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
#endif*/

        bgp_nb_key.family = AF_INET;
        sal_memcpy(&bgp_nb_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }
        
        p_db_bgp_neighbor = tbl_bgp_neighbor_get_bgp_neighbor(&bgp_nb_key);
        if (NULL == p_db_bgp_neighbor)
        {
            /*CTCLIB_SLIST_LOOP_DEL(p_master->bgp_neighbor_list, p_bgp_neighbor, listnode, next)
            {
                if (neighbor_addr_match((addr_t *)&p_bgp_neighbor->key, (addr_t *)&bgp_nb_key))
                {
                    return rc;
                }                    
            }*/
            sal_memcpy(&bgp_nb.key, &bgp_nb_key, sizeof(bgp_nb_key));
            tbl_bgp_neighbor_add_bgp_neighbor(&bgp_nb);
            p_db_bgp_neighbor = tbl_bgp_neighbor_get_bgp_neighbor(&bgp_nb_key);
        }
        switch(field_id)
        {
        case TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP:
            PM_CFG_GET_UINT(ebgp_multihop, argv[2], para->p_rs);
            p_db_bgp_neighbor->ebgp_multihop = ebgp_multihop;
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_EBGP_MULTIHOP);
            break;
        case TBL_BGP_NEIGHBOR_FLD_PASSWORD
:
            if(argc == 2)
            {
                sal_memset(p_db_bgp_neighbor->password, 0, sizeof(bgp_nb.password));                
                tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_PASSWORD);
            }
            else if(argc == 3)
            {
                sal_memcpy(p_db_bgp_neighbor->password, argv[2], sizeof(bgp_nb.password));
                tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_PASSWORD);
            }
            break;
        case TBL_BGP_NEIGHBOR_FLD_INTF_NAME
:
            sal_memset(&p_db_bgp_neighbor->intf_addr, 0, sizeof(bgp_nb.intf_addr));
            sal_memcpy(p_db_bgp_neighbor->intf_name, argv[2], IFNAME_SIZE);
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_INTF_ADDR);
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_INTF_NAME);
            break;
        case TBL_BGP_NEIGHBOR_FLD_INTF_ADDR
:
            retv = sal_inet_pton(AF_INET, argv[2], buff);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid neighbor prefix");
            }
            sal_memset(p_db_bgp_neighbor->intf_name, 0, IFNAME_SIZE);
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_INTF_NAME);
            sal_memcpy(&p_db_bgp_neighbor->intf_addr, buff, sizeof(addr_ipv4_t));
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_INTF_ADDR);
            break;
        case TBL_BGP_NEIGHBOR_FLD_SHUTDOWN
:
            p_db_bgp_neighbor->shutdown = atoi(argv[2]);
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_SHUTDOWN);
            break;
        case TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF:
            p_db_bgp_neighbor->next_hop_self = atoi(argv[2]);
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_NEXT_HOP_SELF);
            break;
        default:
		 	CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
            break;
        }
        break;
	case CDB_OPER_DEL:
        retv = sal_inet_pton(AF_INET, argv[0], buf);
        if (!retv)
        {
            CFG_INVALID_PARAM_RET("Invalid neighbor prefix");
        }

        sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
        /*#ifdef _CENTEC_
        ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
        #endif*/

        bgp_nb_key.family = AF_INET;
        sal_memcpy(&bgp_nb_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
        p_db_bgp_neighbor = tbl_bgp_neighbor_get_bgp_neighbor(&bgp_nb_key);
        if (1 == argc)
        {
            if (NULL != p_db_bgp_neighbor)
            {
                tbl_bgp_neighbor_del_bgp_neighbor(&bgp_nb_key);
            }
        }
        /*else
        {
            p_db_bgp_neighbor->shutdown = 0;
            tbl_bgp_neighbor_set_bgp_neighbor_field(p_db_bgp_neighbor,TBL_BGP_NEIGHBOR_FLD_SHUTDOWN);
        }*/
		break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;

}

int32
rt_cmd_process_rip(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
	tbl_rip_t						*p_db_rip;
    tbl_rip_t                      	rip;
    tbl_rip_key_t                  	rip_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
	
	rip_timers_t 					timers;
		
   
	uint8 		version;
	uint32		update;
	uint32		timeout;
	uint32 		garbage;
	uint32 		value;

    cdb_field_t *p_field            = NULL;
    tbl_rip_t  *p_rip				= NULL;
	int32       source              = 0;
    int32       field_id            = 0;
	uint32      default_information = 0;

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
	tbl_rip_passive_if_t 				*p_rip_ps = NULL;
	tbl_rip_passive_if_master_t 		*p_ps_master = NULL;
    p_ps_master = tbl_rip_passive_if_get_master();

    CFG_DBG_FUNC();

	
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&rip, 0, sizeof(rip));
    sal_memset(&rip_key, 0, sizeof(rip_key));
	sal_memset(&timers, 0, sizeof(rip_timers_t));
    p_node = cdb_get_tbl(TBL_RIP);

    rip_key.id = RIP_DEFAULT_INSTANCE;
    p_rip = tbl_rip_get_rip(&rip_key);

	
	switch (para->oper)
	{
	case CDB_OPER_ADD:
		if (1 == argc && 0 == sal_strcmp(argv[0], "create_rip"))
		{
			if (NULL == p_rip)
			{
				rip.key.id = RIP_DEFAULT_INSTANCE;
				rc = tbl_rip_add_rip(&rip);
			}
		}
		break;	
		
	case CDB_OPER_DEL:
        if (1 == argc && 0 == sal_strcmp(argv[0], "remove_rip"))
        {
            _rt_cmd_del_rip_process();
        }
        break;
		
   	case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (NULL == p_rip)
        {
            rip.key.id = RIP_DEFAULT_INSTANCE;
            rc = tbl_rip_add_rip(&rip);
        }
        p_db_rip = tbl_rip_get_rip(&rip_key);
        
        switch (field_id)
        {
         case TBL_RIP_FLD_VERSION:
			if (2 == argc && 0 == sal_strcmp(argv[0], "version"))
        	{
          		PM_CFG_GET_UINT(version, argv[1], para->p_rs);
               
                if (NULL != p_db_rip)
                {
                    if (0 != sal_memcmp(&p_db_rip->version, &version, sizeof(uint8)))
                    {
                        sal_memcpy(&p_db_rip->version, &version, sizeof(uint8));
                        tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_VERSION);
                    }
                }
			}
			break;
			

		case TBL_RIP_FLD_TIMERS:
			if (4 == argc && 0 == sal_strcmp(argv[0], "timers"))
        	{
          		PM_CFG_GET_UINT(update, argv[1], para->p_rs);
				PM_CFG_GET_UINT(timeout, argv[2], para->p_rs);
				PM_CFG_GET_UINT(garbage, argv[3], para->p_rs);
               
                if (NULL != p_db_rip)
                {
                    if (0 != sal_memcmp(&p_db_rip->timers.update, &update, sizeof(uint32)))
                    {
                        sal_memcpy(&p_db_rip->timers.update, &update, sizeof(uint32));            
                    }
					if (0 != sal_memcmp(&p_db_rip->timers.timeout, &timeout, sizeof(uint32)))
                    {
                        sal_memcpy(&p_db_rip->timers.timeout, &timeout, sizeof(uint32));            
                    }
					if (0 != sal_memcmp(&p_db_rip->timers.garbage, &garbage, sizeof(uint32)))
                    {
                        sal_memcpy(&p_db_rip->timers.garbage, &garbage, sizeof(uint32));            
                    }
					tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_TIMERS);
                }
			}
			break;
			
		case TBL_RIP_FLD_DEFAULT_INFORMATION:
			if (0 == sal_strcmp(argv[1], "add"))
			{
				if (2 == argc)
				{
					if (NULL != p_db_rip)
					{
						default_information = RIP_DEFAULT_INFORMATION;
						if (default_information != p_db_rip->default_information)
						{
							_rt_cmd_clear_rip_default_information(p_db_rip);
							p_db_rip->default_information = default_information;
							tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_DEFAULT_INFORMATION);
						}
					}					
				}
			}
			if (0 == sal_strcmp(argv[1], "del"))
			{
				if (2 == argc)
				{
					if (NULL != p_db_rip)
					{
						default_information = RIP_DEFAULT_INFORMATION_NONE;
						if (default_information != p_db_rip->default_information)
						{
							p_db_rip->default_information = default_information;
							tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_DEFAULT_INFORMATION);
						}
					}
				}
			}
			break;

		case TBL_RIP_FLD_PASSIVE_IF_DEFAULT:
			if (2 == argc && 0 == sal_strcmp(argv[0], "passive_if_default"))
			{
				PM_CFG_GET_UINT(value, argv[1], para->p_rs);
				if (NULL != p_db_rip)
				{
					if (p_db_rip->passive_if_default != value)
					{
						p_db_rip->passive_if_default = value;
						tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_PASSIVE_IF_DEFAULT);
						if (value == 1)
						{
							CTCLIB_SLIST_LOOP_DEL(p_ps_master->rip_passive_if_list, p_rip_ps, listnode, next)
			                {
			                	tbl_rip_passive_if_del_rip_passive_if(&p_rip_ps->key);                  
			                }
						}
					}
				}
			}
			break;

		case TBL_RIP_FLD_REDISTRIBUTE:
			PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            if (0 == sal_strcmp(argv[1], "add"))
            {
                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                if (NULL != p_db_rip)
                {
                    if (!GLB_IS_BIT_SET(p_db_rip->redistribute, source))
                    {
                        GLB_SET_BIT(p_db_rip->redistribute, source);
						tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_REDISTRIBUTE);
                    }
					if (argc>3 && 0 == sal_strcmp(argv[3], "red_metric"))
					{
						PM_CFG_GET_UINT(value, argv[4], para->p_rs);
						p_db_rip->red_metric[source] = value+1;//0-16 -> 1-17
						tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_RED_METRIC);
					}
                }
            }

            if (0 == sal_strcmp(argv[1], "del"))
            {
                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                if (NULL != p_db_rip)
                {
                    if (GLB_IS_BIT_SET(p_db_rip->redistribute, source))
                    {
                        GLB_CLEAR_BIT(p_db_rip->redistribute, source);
						p_db_rip->red_metric[source] = 0;
                        tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_REDISTRIBUTE);
                    }
                }
            }
            break;

		case TBL_RIP_FLD_DEFAULT_METRIC:
			if (2 == argc && 0 == sal_strcmp(argv[0], "default_metric"))
			{
				PM_CFG_GET_UINT(value, argv[1], para->p_rs);
				if (NULL != p_db_rip)
				{
					if (p_db_rip->default_metric != value)
					{
						p_db_rip->default_metric = value;
						tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_DEFAULT_METRIC);
					}
				}			
			}
			
			break;

		case TBL_RIP_FLD_ALLOW_ECMP:
			if (2 == argc && 0 == sal_strcmp(argv[0], "allow_ecmp"))
			{
				if (NULL != p_db_rip)
				{
					if (0 == sal_strcmp(argv[1], "add"))
					{
						p_db_rip->allow_ecmp= 1;
						tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_ALLOW_ECMP);
					}
					else if (0 == sal_strcmp(argv[1], "del"))
					{
						p_db_rip->allow_ecmp= 0;
						tbl_rip_set_rip_field(p_db_rip, TBL_RIP_FLD_ALLOW_ECMP);
					}
				}
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
rt_cmd_process_rip_network(char **argv, int32 argc, cfg_cmd_para_t *para)
{    

    cdb_node_t *p_node              = NULL;
	tbl_rip_network_t              	rip_net;
	tbl_rip_network_key_t			rip_net_key;
	tbl_rip_network_t				*p_db_rip_network = NULL;



    int32       rc		= PM_E_NONE;
    int32       retv 	= 0;
	uint8       masklen = 0;
	char        buf[64];
	addr_ipv4_t ip_prefix;

	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&rip_net_key, 0, sizeof(rip_net_key));
    sal_memset(&rip_net, 0, sizeof(rip_net));
    p_node = cdb_get_tbl(TBL_RIP_NETWORK);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_rip_network_t 				*p_rip_network = NULL;
	tbl_rip_network_master_t 		*p_master = NULL;
	uint32      mask = 0;
    p_master = tbl_rip_network_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (2 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }
		
            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            masklen = sal_atoi(argv[1]);

 
		#ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
		#endif
            rip_net_key.family = AF_INET;
            rip_net_key.prefixlen = masklen;
            sal_memcpy(&rip_net_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_rip_network = tbl_rip_network_get_rip_network(&rip_net_key);
            if (NULL == p_db_rip_network)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->rip_network_list, p_rip_network, listnode, next)
                {
                    if (sal_prefix_match((prefix_t *)&p_rip_network->key, (prefix_t *)&rip_net_key) 
                        && p_rip_network->key.prefixlen == rip_net_key.prefixlen)
                    {
                        return rc;
                    }                    
                }
                sal_memcpy(&rip_net.key, &rip_net_key, sizeof(rip_net_key));
                tbl_rip_network_add_rip_network(&rip_net);
            }
        }
        break;
		
	case CDB_OPER_DEL:
		if (2 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            masklen = sal_atoi(argv[1]);
            #ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif

            rip_net_key.family = AF_INET;
            rip_net_key.prefixlen = masklen;
            sal_memcpy(&rip_net_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_rip_network = tbl_rip_network_get_rip_network(&rip_net_key);
            if (NULL != p_db_rip_network)
            {
                tbl_rip_network_del_rip_network(&rip_net_key);
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
rt_cmd_process_ripng_network(char **argv, int32 argc, cfg_cmd_para_t *para)
{    

    cdb_node_t *p_node              = NULL;
	tbl_ripng_network_t              	ripng_net;
	tbl_ripng_network_key_t			ripng_net_key;
	tbl_ripng_network_t				*p_db_ripng_network = NULL;



    int32       rc		= PM_E_NONE;
    int32       retv 	= 0;
	uint8       masklen = 0;
	char        buf[64];
	addr_ipv6_t ip_prefix;
	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&ripng_net_key, 0, sizeof(ripng_net_key));
    sal_memset(&ripng_net, 0, sizeof(ripng_net));
    p_node = cdb_get_tbl(TBL_RIPNG_NETWORK);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_ripng_network_t 				*p_ripng_network = NULL;
	tbl_ripng_network_master_t 		*p_master = NULL;
	uint32      mask = 0;
    p_master = tbl_ripng_network_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (2 == argc)
        {
            retv = sal_inet_pton(AF_INET6, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv6_t));
            masklen = sal_atoi(argv[1]);

#if 0
		#ifdef _CENTEC_
            IPV6_LEN_TO_MASK(mask, masklen);
            ip_prefix.s6_addr = sal_htonl(ip_prefix.s6_addr);
            ip_prefix.s6_addr = ip_prefix.s6_addr & mask;
            ip_prefix.s6_addr = sal_htonl(ip_prefix.s6_addr);
		#endif
#endif
            ripng_net_key.family = AF_INET6;
            ripng_net_key.prefixlen = masklen;
            sal_memcpy(&ripng_net_key.u.prefix6, &ip_prefix, sizeof(addr_ipv6_t));
            p_db_ripng_network = tbl_ripng_network_get_ripng_network(&ripng_net_key);
            if (NULL == p_db_ripng_network)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->ripng_network_list, p_ripng_network, listnode, next)
                {
                   // if (prefix_match((prefix_t *)&p_ripng_network->key, (prefix_t *)&ripng_net_key) 
					if (0 == IPV6_ADDR_CMP(&(p_ripng_network->key.u.prefix6), &(ripng_net_key.u.prefix6))
                        && p_ripng_network->key.prefixlen == ripng_net_key.prefixlen)
                    {
                        return rc;
                    }                    
                }

                sal_memcpy(&ripng_net.key, &ripng_net_key, sizeof(ripng_net_key));
                tbl_ripng_network_add_ripng_network(&ripng_net);
            }
        }
        break;
		
	case CDB_OPER_DEL:
		if (2 == argc)
        {
            retv = sal_inet_pton(AF_INET6, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv6_t));
            masklen = sal_atoi(argv[1]);
#if 0
            #ifdef _CENTEC_
            IPV6_LEN_TO_MASK(mask, masklen);
            ip_prefix.s6_addr = sal_htonl(ip_prefix.s6_addr);
            ip_prefix.s6_addr = ip_prefix.s6_addr & mask;
            ip_prefix.s6_addr = sal_htonl(ip_prefix.s6_addr);
            #endif
#endif
            ripng_net_key.family = AF_INET6;
            ripng_net_key.prefixlen = masklen;
            sal_memcpy(&ripng_net_key.u.prefix6, &ip_prefix, sizeof(addr_ipv6_t));
            p_db_ripng_network = tbl_ripng_network_get_ripng_network(&ripng_net_key);
            if (NULL != p_db_ripng_network)
            {
                tbl_ripng_network_del_ripng_network(&ripng_net_key);
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
rt_cmd_process_rip_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para)
{    

    cdb_node_t *p_node              = NULL;
	tbl_rip_neighbor_t             	rip_nb;
	tbl_rip_neighbor_key_t			rip_nb_key;
	tbl_rip_neighbor_t				*p_db_rip_neighbor = NULL;



    int32       rc		= PM_E_NONE;
    int32       retv 	= 0;
	char        buf[64];
	addr_ipv4_t ip_prefix;

	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&rip_nb_key, 0, sizeof(rip_nb_key));
    sal_memset(&rip_nb, 0, sizeof(rip_nb));
    p_node = cdb_get_tbl(TBL_RIP_NEIGHBOR);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_rip_neighbor_t 				*p_rip_neighbor = NULL;
	tbl_rip_neighbor_master_t 		*p_master = NULL;
    p_master = tbl_rip_neighbor_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid neighbor prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));

			#ifdef _CENTEC_
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
			#endif

            rip_nb_key.family = AF_INET;
            sal_memcpy(&rip_nb_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_rip_neighbor = tbl_rip_neighbor_get_rip_neighbor(&rip_nb_key);
            if (NULL == p_db_rip_neighbor)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->rip_neighbor_list, p_rip_neighbor, listnode, next)
                {
                    if (neighbor_addr_match((addr_t *)&p_rip_neighbor->key, (addr_t *)&rip_nb_key))
                    {
                        return rc;
                    }                    
                }

                sal_memcpy(&rip_nb.key, &rip_nb_key, sizeof(rip_nb_key));
                tbl_rip_neighbor_add_rip_neighbor(&rip_nb);
            }
        }
        break;
		
	case CDB_OPER_DEL:
		if (1 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid neighbor prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            #ifdef _CENTEC_
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif

            rip_nb_key.family = AF_INET;
            sal_memcpy(&rip_nb_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_rip_neighbor = tbl_rip_neighbor_get_rip_neighbor(&rip_nb_key);
            if (NULL != p_db_rip_neighbor)
            {
                tbl_rip_neighbor_del_rip_neighbor(&rip_nb_key);
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
rt_cmd_process_rip_passive_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
	cdb_node_t *p_node              = NULL;
	tbl_rip_passive_if_t            rip_ps;
	tbl_rip_passive_if_key_t		rip_ps_key;
	tbl_rip_passive_if_t			*p_db_rip_ps = NULL;
	tbl_rip_key_t                  	rip_key;

	tbl_rip_t  *p_rip				= NULL;
	rip_key.id = RIP_DEFAULT_INSTANCE;
    p_rip = tbl_rip_get_rip(&rip_key);

    int32       rc		= PM_E_NONE;
	char        buf[64];

	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&rip_ps_key, 0, sizeof(rip_ps_key));
    sal_memset(&rip_ps, 0, sizeof(rip_ps));
    p_node = cdb_get_tbl(TBL_RIP_PASSIVE_IF);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_rip_passive_if_t 				*p_rip_ps = NULL;
	tbl_rip_passive_if_master_t 		*p_master = NULL;
    p_master = tbl_rip_passive_if_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
		if (p_rip->passive_if_default == 1)
		{
			return rc;
		}
        if (1 == argc)
        {
   			sal_strncpy(rip_ps_key.name, argv[0], IFNAME_SIZE);
            p_db_rip_ps = tbl_rip_passive_if_get_rip_passive_if(&rip_ps_key);
            if (NULL == p_db_rip_ps)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->rip_passive_if_list, p_rip_ps, listnode, next)
                {
                    if (0 == sal_memcmp(p_rip_ps->key.name, rip_ps_key.name,sizeof(p_rip_ps->key.name)))
                    {
                        return rc;
                    }                    
                }

                sal_memcpy(&rip_ps.key, &rip_ps_key, sizeof(rip_ps_key));
                tbl_rip_passive_if_add_rip_passive_if(&rip_ps);
            }
        }
        break;
		
	case CDB_OPER_DEL:
		if (1 == argc)
        {
   			sal_strncpy(rip_ps_key.name, argv[0], IFNAME_SIZE);
			
            p_db_rip_ps = tbl_rip_passive_if_get_rip_passive_if(&rip_ps_key);
            if (NULL != p_db_rip_ps)
            {
                tbl_rip_passive_if_del_rip_passive_if(&rip_ps_key);
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
rt_cmd_process_ripng_passive_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
	cdb_node_t *p_node              = NULL;
	tbl_ripng_passive_if_t            ripng_ps;
	tbl_ripng_passive_if_key_t		ripng_ps_key;
	tbl_ripng_passive_if_t			*p_db_ripng_ps = NULL;
	tbl_ripng_key_t                  	ripng_key;

	tbl_ripng_t  *p_ripng				= NULL;
	ripng_key.id = RIPNG_DEFAULT_INSTANCE;
    p_ripng = tbl_ripng_get_ripng(&ripng_key);

    int32       rc		= PM_E_NONE;
	char        buf[64];

	
	CFG_DBG_FUNC();
	
	sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ripng_ps_key, 0, sizeof(ripng_ps_key));
    sal_memset(&ripng_ps, 0, sizeof(ripng_ps));
    p_node = cdb_get_tbl(TBL_RIPNG_PASSIVE_IF);
	

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
    tbl_ripng_passive_if_t 				*p_ripng_ps = NULL;
	tbl_ripng_passive_if_master_t 		*p_master = NULL;
    p_master = tbl_ripng_passive_if_get_master();
	

    if (NULL == p_master)
    {
        return rc;
    }


    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 == argc)
        {
   			sal_strncpy(ripng_ps_key.name, argv[0], IFNAME_SIZE);
            p_db_ripng_ps = tbl_ripng_passive_if_get_ripng_passive_if(&ripng_ps_key);
            if (NULL == p_db_ripng_ps)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->ripng_passive_if_list, p_ripng_ps, listnode, next)
                {
                    if (0 == sal_memcmp(p_ripng_ps->key.name, ripng_ps_key.name,sizeof(p_ripng_ps->key.name)))
                    {
                        return rc;
                    }                    
                }

                sal_memcpy(&ripng_ps.key, &ripng_ps_key, sizeof(ripng_ps_key));
                tbl_ripng_passive_if_add_ripng_passive_if(&ripng_ps);
            }
        }
        break;
		
	case CDB_OPER_DEL:
		if (1 == argc)
        {
   			sal_strncpy(ripng_ps_key.name, argv[0], IFNAME_SIZE);
			
            p_db_ripng_ps = tbl_ripng_passive_if_get_ripng_passive_if(&ripng_ps_key);
            if (NULL != p_db_ripng_ps)
            {
                tbl_ripng_passive_if_del_ripng_passive_if(&ripng_ps_key);
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
rt_cmd_process_ripng(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
	tbl_ripng_t						*p_db_ripng;
    tbl_ripng_t                      	ripng;
    tbl_ripng_key_t                  	ripng_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
	
	rip_timers_t 					timers;
		
   
	uint8 		version;
	uint32		update;
	uint32		timeout;
	uint32 		garbage;
	uint32 		value;

    cdb_field_t *p_field            = NULL;
    tbl_ripng_t  *p_ripng				= NULL;
	int32       source              = 0;
    int32       field_id            = 0;
	uint32      default_information = 0;

	ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
	
	tbl_ripng_passive_if_t 				*p_ripng_ps = NULL;
	tbl_ripng_passive_if_master_t 		*p_ps_master = NULL;
    p_ps_master = tbl_ripng_passive_if_get_master();

    CFG_DBG_FUNC();

	
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ripng, 0, sizeof(ripng));
    sal_memset(&ripng_key, 0, sizeof(ripng_key));
	sal_memset(&timers, 0, sizeof(rip_timers_t));
    p_node = cdb_get_tbl(TBL_RIPNG);

    ripng_key.id = RIPNG_DEFAULT_INSTANCE;
    p_ripng = tbl_ripng_get_ripng(&ripng_key);

	switch (para->oper)
	{
	case CDB_OPER_ADD:							   
		if (1 == argc && 0 == sal_strcmp(argv[0], "create_ripng"))
		{
			if (NULL == p_ripng)
			{
				ripng.key.id = RIPNG_DEFAULT_INSTANCE;
				rc = tbl_ripng_add_ripng(&ripng);
			}
		}
		break;	
		
	case CDB_OPER_DEL:
        if (1 == argc && 0 == sal_strcmp(argv[0], "remove_ripng"))
        {
            _rt_cmd_del_ripng_process();
        }
        break;
		
   	case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (NULL == p_ripng)
        {
            ripng.key.id = RIPNG_DEFAULT_INSTANCE;
            rc = tbl_ripng_add_ripng(&ripng);
        }
        p_db_ripng = tbl_ripng_get_ripng(&ripng_key);
        
        switch (field_id)
        {
		case TBL_RIPNG_FLD_TIMERS:
			if (4 == argc && 0 == sal_strcmp(argv[0], "timers"))
        	{
          		PM_CFG_GET_UINT(update, argv[1], para->p_rs);
				PM_CFG_GET_UINT(timeout, argv[2], para->p_rs);
				PM_CFG_GET_UINT(garbage, argv[3], para->p_rs);
               
                if (NULL != p_db_ripng)
                {
                    if (0 != sal_memcmp(&p_db_ripng->timers.update, &update, sizeof(uint32)))
                    {
                        sal_memcpy(&p_db_ripng->timers.update, &update, sizeof(uint32));            
                    }
					if (0 != sal_memcmp(&p_db_ripng->timers.timeout, &timeout, sizeof(uint32)))
                    {
                        sal_memcpy(&p_db_ripng->timers.timeout, &timeout, sizeof(uint32));            
                    }
					if (0 != sal_memcmp(&p_db_ripng->timers.garbage, &garbage, sizeof(uint32)))
                    {
                        sal_memcpy(&p_db_ripng->timers.garbage, &garbage, sizeof(uint32));            
                    }
					tbl_rip_set_rip_field(p_db_ripng, TBL_RIPNG_FLD_TIMERS);
                }
			}
			break;
			
		case TBL_RIPNG_FLD_DEFAULT_INFORMATION:
			if (0 == sal_strcmp(argv[1], "add"))
			{
				if (2 == argc)
				{
					if (NULL != p_db_ripng)
					{
						default_information = RIPNG_DEFAULT_INFORMATION;
						if (default_information != p_db_ripng->default_information)
						{
							_rt_cmd_clear_ripng_default_information(p_db_ripng);
							p_db_ripng->default_information = default_information;
							tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_DEFAULT_INFORMATION);
						}
					}					
				}
			}
			if (0 == sal_strcmp(argv[1], "del"))
			{
				if (2 == argc)
				{
					if (NULL != p_db_ripng)
					{
						default_information = RIPNG_DEFAULT_INFORMATION_NONE;
						if (default_information != p_db_ripng->default_information)
						{
							p_db_ripng->default_information = default_information;
							tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_DEFAULT_INFORMATION);
						}
					}
				}
			}
			break;	

		case TBL_RIPNG_FLD_REDISTRIBUTE:
			PM_CFG_CHECK_MIN_PARAMS(argc, 3, para->p_rs);
            if (0 == sal_strcmp(argv[1], "add"))
            {
                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                if (NULL != p_db_ripng)
                {
                    if (!GLB_IS_BIT_SET(p_db_ripng->redistribute, source))
                    {
                        GLB_SET_BIT(p_db_ripng->redistribute, source);
						tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_REDISTRIBUTE);
                    }
					if (argc>3 && 0 == sal_strcmp(argv[3], "red_metric"))
					{
						PM_CFG_GET_UINT(value, argv[4], para->p_rs);
						p_db_ripng->red_metric[source] = value+1;//0-16 -> 1-17
						tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_RED_METRIC);
					}
                }
            }

            if (0 == sal_strcmp(argv[1], "del"))
            {
                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                if (NULL != p_db_ripng)
                {
                    if (GLB_IS_BIT_SET(p_db_ripng->redistribute, source))
                    {
                        GLB_CLEAR_BIT(p_db_ripng->redistribute, source);
						p_db_ripng->red_metric[source] = 0;
                        tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_REDISTRIBUTE);
                    }
                }
            }
            break;

		case TBL_RIPNG_FLD_DEFAULT_METRIC:
			if (2 == argc && 0 == sal_strcmp(argv[0], "default_metric"))
			{
				PM_CFG_GET_UINT(value, argv[1], para->p_rs);
				if (NULL != p_db_ripng)
				{
					if (p_db_ripng->default_metric != value)
					{
						p_db_ripng->default_metric = value;
						tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_DEFAULT_METRIC);
					}
				}			
			}
			
			break;

		case TBL_RIPNG_FLD_ALLOW_ECMP:
			if (2 == argc && 0 == sal_strcmp(argv[0], "allow_ecmp"))
			{
				if (NULL != p_db_ripng)
				{
					if (0 == sal_strcmp(argv[1], "add"))
					{
						p_db_ripng->allow_ecmp= 1;
						tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_ALLOW_ECMP);
					}
					else if (0 == sal_strcmp(argv[1], "del"))
					{
						p_db_ripng->allow_ecmp= 0;
						tbl_ripng_set_ripng_field(p_db_ripng, TBL_RIPNG_FLD_ALLOW_ECMP);
					}
				}
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
rt_cmd_process_ospf(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
    tbl_ospf_t                      *p_db_ospf = NULL;
    tbl_ospf_t                      ospf;
    tbl_ospf_key_t                  ospf_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
    int32       retv = 0;
    char        buf[64];
    addr_ipv4_t routerid;
    cdb_field_t *p_field            = NULL;
    tbl_ospf_t  *p_ospf             = NULL;
    int32       source              = 0;
    int32       field_id            = 0;
    uint32      default_originate   = 0;
    char rid_check[64];    
    char filebuf[256];
    char cmd_str[128] = {0};
    int32   remained = 0;    /* modified by liwh for bug 51014, 2019-03-28 */

    CFG_DBG_FUNC();

    sal_memset(rid_check, 0, sizeof(rid_check));
    sal_memset(filebuf, 0, sizeof(filebuf));

    sal_memset(&routerid, 0, sizeof(routerid));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf, 0, sizeof(ospf));
    sal_memset(&ospf_key, 0, sizeof(ospf_key));
    p_node = cdb_get_tbl(TBL_OSPF);

    ospf_key.id = OSPF_DEFAULT_INSTANCE;
    p_ospf = tbl_ospf_get_ospf(&ospf_key);
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 == argc && 0 == sal_strcmp(argv[0], "create_ospf"))
        {
            if (NULL == p_ospf)
            {
                ospf.key.id = OSPF_DEFAULT_INSTANCE;
                rc = tbl_ospf_add_ospf(&ospf);
                if (PM_E_NONE == rc)
                {
                    sal_memset(cmd_str, 0, sizeof(cmd_str));
                    rt_vty_router_ospf_cmd("");
                }
            }
        }
        break;

    case CDB_OPER_DEL:
        /* modified by liwh for bug 51014, 2019-03-28 */
        rt_clear_ospf_timer(p_ospf);
        /* liwh */
        
        if (1 == argc && 0 == sal_strcmp(argv[0], "remove_ospf"))
        {
            _rt_cmd_del_ospf_process();
        }
        break;

    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (NULL == p_ospf)
        {
            /* modified by liwh for bug 44373, 2017-06-20 */
            ospf.key.id = OSPF_DEFAULT_INSTANCE;
            /* liwh end */
            rc = tbl_ospf_add_ospf(&ospf);
        }
        p_db_ospf = tbl_ospf_get_ospf(&ospf_key);
        
        switch (field_id)
        {
            case TBL_OSPF_FLD_ROUTER_ID:
                if (2 == argc && 0 == sal_strcmp(argv[0], "router_id"))
                {
                    retv = sal_inet_pton(AF_INET, argv[1], buf);
                    if (!retv)
                    {
                        CFG_INVALID_PARAM_RET("Invalid router-id");
                    }

                    if (NULL != p_db_ospf)
                    {
                        sal_memcpy(&routerid, buf, sizeof(addr_ipv4_t));
                        if (0 == routerid.s_addr)
                        {
                            CFG_INVALID_PARAM_RET("Invalid router-id");
                        }
                        if (0 != sal_memcmp(&p_db_ospf->router_id, &routerid, sizeof(addr_ipv4_t)))
                        {
                            sal_memcpy(&p_db_ospf->router_id, &routerid, sizeof(addr_ipv4_t));
                            tbl_ospf_set_ospf_field(p_db_ospf, TBL_OSPF_FLD_ROUTER_ID);
                            /* modified by liwh for bug 44589, 2017-07-10 */
                            //_rt_cmd_set_ospf_router_id(p_db_ospf);
                            /*liwh end */

                            #if 0

                            sal_snprintf(rid_check, sizeof(rid_check), " router-id %s", argv[1]);
                            startup_config_fd = sal_fopen("/mnt/flash/startup-config.conf", "r");
                            if (startup_config_fd)
                            {
                                while (sal_fgets(filebuf, 128, startup_config_fd))
                                {   
                                    if (!sal_strncmp(filebuf, rid_check, sal_strlen(rid_check)))
                                    {
                                        rid_upd = TRUE;
                                        break;
                                    }
                                }
                                sal_fclose(startup_config_fd);
                            }

                            if (rid_upd)
                            {
                                _rt_cmd_set_ospf_router_id(p_db_ospf);
                            }
                            else
                            {
                                CFG_PROMPT_RET("Configuration of OSPF router-id differs from startup-config, need to be stored and take effect until the next reload");
                            }
                            #endif
                    
                            sal_memset(cmd_str, 0, sizeof(cmd_str));
                            sal_sprintf(cmd_str, "router-id %s", argv[1]);
                            rc = rt_vty_router_ospf_cmd(cmd_str);
                            /* modified by liwh for bug 47215, 2018-09-11 */
                            if (rc)
                            {
                                CFG_CONFLICT_RET("Ospf router-id can't take effect until clear ip ospf process");    
                            }
                            /* liwh end */
                        }
                    }
                }
                if (1 == argc && 0 == sal_strcmp(argv[0], "router_id"))
                {
                    if (NULL != p_db_ospf)
                    {
                        sal_memset(&p_db_ospf->router_id, 0, sizeof(addr_ipv4_t));
                        tbl_ospf_set_ospf_field(p_db_ospf, TBL_OSPF_FLD_ROUTER_ID);
                        /* modified by liwh for bug 44589, 2017-07-10 */
                        //_rt_cmd_set_ospf_router_id(p_db_ospf);
                        /*liwh end */
                        //CFG_PROMPT_RET("Configuration of OSPF router-id differs from startup-config, need to be stored and take effect until the next reload");
                        
                        sal_memset(cmd_str, 0, sizeof(cmd_str));
                        sal_sprintf(cmd_str, "no ospf router-id");
                        rt_vty_router_ospf_cmd(cmd_str);
                    }
                }

                break;

            case TBL_OSPF_FLD_DEFAULT_ORIGINATE:
                if (0 == sal_strcmp(argv[1], "add"))
                {
                    if (3 == argc)
                    {
                        if (0 == sal_strcmp(argv[2], "always"))
                        {
                            default_originate = OSPF_DEFAULT_ORIGINATE_ALWAYS;
                        }
                    }
                    else if ( 2 == argc)
                    {
                        default_originate = OSPF_DEFAULT_ORIGINATE_ZEBRA;
                    }

                    if (NULL != p_db_ospf)
                    {
                        if (default_originate != p_db_ospf->default_originate)
                        {
                            _rt_cmd_clear_ospf_default_info_originate(p_db_ospf);
                            p_db_ospf->default_originate = default_originate;
                            tbl_ospf_set_ospf_field(p_db_ospf, TBL_OSPF_FLD_DEFAULT_ORIGINATE);
                        
                            sal_memset(cmd_str, 0, sizeof(cmd_str));
                            if (3 == argc)
                            {
                                sal_sprintf(cmd_str, "default-information originate always");
                            }
                            else if (2 == argc)
                            {
                                sal_sprintf(cmd_str, "default-information originate");
                            }
                            rt_vty_router_ospf_cmd(cmd_str);
                        }
                    }
                }
                if (0 == sal_strcmp(argv[1], "del"))
                {
                    if (2 == argc)
                    {
                        if (NULL != p_db_ospf)
                        {
                            default_originate = OSPF_DEFAULT_ORIGINATE_NONE;
                            if (default_originate != p_db_ospf->default_originate)
                            {
                                p_db_ospf->default_originate = default_originate;
                                tbl_ospf_set_ospf_field(p_db_ospf, TBL_OSPF_FLD_DEFAULT_ORIGINATE);
                        
                                sal_memset(cmd_str, 0, sizeof(cmd_str));
                                sal_sprintf(cmd_str, "no default-information originate");
                                rt_vty_router_ospf_cmd(cmd_str);
                            }
                        }
                    }
                }
                break;

            case TBL_OSPF_FLD_REDISTRIBUTE:
                PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
                 /* modified by liwh for bug 51014, 2019-03-28 */
                if (RT_ROUTE_CONNECT == source)
                {
                    if (!p_db_ospf->t_ospf_redistribute_connected_timer)
                    {
                        p_db_ospf->t_ospf_redistribute_connected_timer = ctc_task_add_timer(TASK_PRI_NORMAL, rt_ospf_redistribute_connected_timer, p_db_ospf, GLB_OSPF_REDISTRIBUTE_CONNECTED_TIME);
                    }
                    else
                    {
                        remained =ctc_timer_get_remain_sec(p_db_ospf->t_ospf_redistribute_connected_timer);
                        CFG_INVALID_PARAM_RET("Please wait %d seconds when change ospf redistribute connected again", remained);
                    }
                }
  
                if (RT_ROUTE_STATIC == source)
                {
                    if (!p_db_ospf->t_ospf_redistribute_static_timer)
                    {
                        p_db_ospf->t_ospf_redistribute_static_timer = ctc_task_add_timer(TASK_PRI_NORMAL, rt_ospf_redistribute_static_timer, p_db_ospf, GLB_OSPF_REDISTRIBUTE_STATIC_TIME);
                    }
                    else
                    {
                        remained =ctc_timer_get_remain_sec(p_db_ospf->t_ospf_redistribute_static_timer);
                        CFG_INVALID_PARAM_RET("Please wait %d seconds when change ospf redistribute static again", remained);
                    }
                }
                /* liwh end */
                            
                if (0 == sal_strcmp(argv[1], "add"))
                { 
                    if (NULL != p_db_ospf)
                    {
                        if (!GLB_IS_BIT_SET(p_db_ospf->redistribute, source))
                        {
                           
                            
                            GLB_SET_BIT(p_db_ospf->redistribute, source);
                            tbl_ospf_set_ospf_field(p_db_ospf, TBL_OSPF_FLD_REDISTRIBUTE);
                        
                            sal_memset(cmd_str, 0, sizeof(cmd_str));
                            sal_sprintf(cmd_str, "redistribute %s ", argv[2]);
                            rt_vty_router_ospf_cmd(cmd_str);
                        }
                    }
                }

                if (0 == sal_strcmp(argv[1], "del"))
                {
                    if (NULL != p_db_ospf)
                    {
                        if (GLB_IS_BIT_SET(p_db_ospf->redistribute, source))
                        {                            
                            GLB_CLEAR_BIT(p_db_ospf->redistribute, source);
                            tbl_ospf_set_ospf_field(p_db_ospf, TBL_OSPF_FLD_REDISTRIBUTE);
                        
                            sal_memset(cmd_str, 0, sizeof(cmd_str));
                            sal_sprintf(cmd_str, "no redistribute %s ", argv[2]);
                            rt_vty_router_ospf_cmd(cmd_str);
                        }
                    }
                }
                break;

            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
                break;
        }

        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_ospf(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_ospf6(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
    tbl_ospf6_t                      *p_db_ospf6 = NULL;
    tbl_ospf6_t                      ospf6;
    tbl_ospf6_key_t                  ospf6_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
    int32       retv = 0;
    char        buf[64];
    addr_ipv4_t routerid;
    cdb_field_t *p_field            = NULL;
    tbl_ospf6_t  *p_ospf6             = NULL;
    int32       source              = 0;
    int32       field_id            = 0;
    uint32      default_originate   = 0;
    char rid_check[64];    
    char filebuf[256];
    char cmd_str[128] = {0};
    int32   remained = 0;    /* modified by liwh for bug 51014, 2019-03-28 */

    CFG_DBG_FUNC();

    sal_memset(rid_check, 0, sizeof(rid_check));
    sal_memset(filebuf, 0, sizeof(filebuf));

    sal_memset(&routerid, 0, sizeof(routerid));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf6, 0, sizeof(ospf6));
    sal_memset(&ospf6_key, 0, sizeof(ospf6_key));
    p_node = cdb_get_tbl(TBL_OSPF6);

    ospf6_key.id = OSPF_DEFAULT_INSTANCE;
    p_ospf6 = tbl_ospf6_get_ospf6(&ospf6_key);
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (1 == argc && 0 == sal_strcmp(argv[0], "create_ospf6"))
        {
            if (NULL == p_ospf6)
            {
                ospf6.key.id = OSPF_DEFAULT_INSTANCE;
                rc = tbl_ospf6_add_ospf6(&ospf6);
                if (PM_E_NONE != rc)
                {
                    CFG_FUNC_ERR_RET("Create ospf6 failed!");
                }
            }
        }
        break;

    case CDB_OPER_DEL:
        /* modified by liwh for bug 51014, 2019-03-28 */
        //rt_clear_ospf6_timer(p_ospf6);
        /* liwh */
        
        if (1 == argc && 0 == sal_strcmp(argv[0], "remove_ospf6"))
        {
            _rt_cmd_del_ospf6_process();
        }
        break;
		
    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (NULL == p_ospf6)
        {
            ospf6.key.id = OSPF_DEFAULT_INSTANCE;
            rc = tbl_ospf6_add_ospf6(&ospf6);
        }
        p_db_ospf6 = tbl_ospf6_get_ospf6(&ospf6_key);
        
        switch (field_id)
        {
            case TBL_OSPF6_FLD_ROUTER_ID:
                if (2 == argc && 0 == sal_strcmp(argv[0], "router_id"))
                {
                    retv = sal_inet_pton(AF_INET, argv[1], buf);
                    if (!retv)
                    {
                        CFG_INVALID_PARAM_RET("Invalid router-id");
                    }

                    if (NULL != p_db_ospf6)
                    {
                        sal_memcpy(&routerid, buf, sizeof(addr_ipv4_t));
                        if (0 == routerid.s_addr)
                        {
                            CFG_INVALID_PARAM_RET("Invalid router-id");
                        }
                        if (0 != sal_memcmp(&p_db_ospf6->router_id, &routerid, sizeof(addr_ipv4_t)))
                        {
                            sal_memcpy(&p_db_ospf6->router_id, &routerid, sizeof(addr_ipv4_t));
                            tbl_ospf6_set_ospf6_field(p_db_ospf6, TBL_OSPF6_FLD_ROUTER_ID);
                        }
                    }
                }
                if (1 == argc && 0 == sal_strcmp(argv[0], "router_id"))
                {
                    if (NULL != p_db_ospf6)
                    {
                        sal_memset(&p_db_ospf6->router_id, 0, sizeof(addr_ipv4_t));
                        tbl_ospf6_set_ospf6_field(p_db_ospf6, TBL_OSPF6_FLD_ROUTER_ID);
                    }
                }

                break;

            case TBL_OSPF_FLD_REDISTRIBUTE:
                PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);

                source = _rt_cmd_proto_redistnum(AF_INET, argv[2]);
#if 0
                 /* modified by liwh for bug 51014, 2019-03-28 */
                if (RT_ROUTE_CONNECT == source)
                {
                    if (!p_db_ospf->t_ospf_redistribute_connected_timer)
                    {
                        p_db_ospf->t_ospf_redistribute_connected_timer = ctc_task_add_timer(TASK_PRI_NORMAL, rt_ospf_redistribute_connected_timer, p_db_ospf, GLB_OSPF_REDISTRIBUTE_CONNECTED_TIME);
                    }
                    else
                    {
                        remained =ctc_timer_get_remain_sec(p_db_ospf->t_ospf_redistribute_connected_timer);
                        CFG_INVALID_PARAM_RET("Please wait %d seconds when change ospf redistribute connected again", remained);
                    }
                }
  
                if (RT_ROUTE_STATIC == source)
                {
                    if (!p_db_ospf->t_ospf_redistribute_static_timer)
                    {
                        p_db_ospf->t_ospf_redistribute_static_timer = ctc_task_add_timer(TASK_PRI_NORMAL, rt_ospf_redistribute_static_timer, p_db_ospf, GLB_OSPF_REDISTRIBUTE_STATIC_TIME);
                    }
                    else
                    {
                        remained =ctc_timer_get_remain_sec(p_db_ospf->t_ospf_redistribute_static_timer);
                        CFG_INVALID_PARAM_RET("Please wait %d seconds when change ospf redistribute static again", remained);
                    }
                }
                /* liwh end */
#endif
                if (0 == sal_strcmp(argv[1], "add"))
                { 
                    if (NULL != p_db_ospf6)
                    {
                        if (!GLB_IS_BIT_SET(p_db_ospf6->redistribute, source))
                        {
                           
                            
                            GLB_SET_BIT(p_db_ospf6->redistribute, source);
                            tbl_ospf6_set_ospf6_field(p_db_ospf6, TBL_OSPF6_FLD_REDISTRIBUTE);
                        }
                    }
                }

                if (0 == sal_strcmp(argv[1], "del"))
                {
                    if (NULL != p_db_ospf6)
                    {
                        if (GLB_IS_BIT_SET(p_db_ospf6->redistribute, source))
                        {                            
                            GLB_CLEAR_BIT(p_db_ospf6->redistribute, source);
                            tbl_ospf6_set_ospf6_field(p_db_ospf6, TBL_OSPF6_FLD_REDISTRIBUTE);
                        }
                    }
                }
                break;
				
            default:
                CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[0]);
                break;
        }

        break;
		
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_ospf6(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return rc;
}

int32
rt_cmd_process_ospf6_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
    tbl_ospf6_interface_t           ospf6_interface;
    tbl_ospf6_interface_key_t       ospf6_interface_key;
    int32                           rc    = PM_E_NONE;
    char        buf[64];
	addr_ipv4_t area_id;
    tbl_ospf6_interface_t  *p_ospf6_interface             = NULL;

    CFG_DBG_FUNC();

    sal_memset(&area_id, 0, sizeof(area_id));
    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ospf6_interface, 0, sizeof(ospf6_interface));
    sal_memset(&ospf6_interface_key, 0, sizeof(ospf6_interface_key));

	sal_memcpy(ospf6_interface_key.name, argv[0], IFNAME_SIZE);
    p_ospf6_interface = tbl_ospf6_interface_get_ospf6_interface(&ospf6_interface_key);
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (NULL == p_ospf6_interface)
        {
            sal_memcpy(ospf6_interface.key.name, argv[0], IFNAME_SIZE);
			sal_memcpy(ospf6_interface.interface, argv[0], IFNAME_SIZE);

			sal_inet_pton(AF_INET, argv[2], buf);
            sal_memcpy(&ospf6_interface.area_id, buf, sizeof(addr_ipv4_t));
            rc = tbl_ospf6_interface_add_ospf6_interface(&ospf6_interface);
        }
		else
		{
			CFG_INVALID_PARAM_RET("interface %s is already configured!", argv[0]);
		}
        break;

    case CDB_OPER_DEL:
       if (NULL == p_ospf6_interface)
       {
			CFG_INVALID_PARAM_RET("interface %s have no configure!", argv[0]);
	   }
	  else
	  {
			tbl_ospf6_interface_del_ospf6_interface(p_ospf6_interface);
	  }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return rc;
}


int32
rt_cmd_process_ospf_network(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
    tbl_ospf_network_t              *p_db_ospf_network = NULL;
    tbl_ospf_network_t              ospf_net;
    tbl_ospf_network_key_t          ospf_net_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
    int32       retv    = 0;
    uint8       masklen = 0;
    uint32      areaid  = 0;
    char        buf[64];
    addr_ipv4_t ip_prefix;
    char cmd_str[128] = {0};

    CFG_DBG_FUNC();

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&ospf_net_key, 0, sizeof(ospf_net_key));
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf_net, 0, sizeof(ospf_net));
    p_node = cdb_get_tbl(TBL_OSPF_NETWORK);

    tbl_ospf_t                      *p_db_ospf = NULL;
    tbl_ospf_t                      ospf;
    tbl_ospf_key_t                  ospf_key;
    
    sal_memset(&ospf, 0, sizeof(ospf));
    sal_memset(&ospf_key, 0, sizeof(ospf_key));

    ospf_key.id = OSPF_DEFAULT_INSTANCE;
    p_db_ospf = tbl_ospf_get_ospf(&ospf_key);
    if (NULL == p_db_ospf)
    {
        rc = tbl_ospf_add_ospf(&ospf);
    }

    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ospf_network_t *p_ospf_network = NULL;
    tbl_ospf_network_master_t *p_master = NULL;
    p_master = tbl_ospf_network_get_master();
    uint32      mask = 0;

    if (NULL == p_master)
    {
        return rc;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (3 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            /* modified by liwh for bug 44506, 2017-06-28 */
            if (p_master->ospf_network_list->count >= GLB_OSPF_NETWORK_MAX)
            {
                CFG_INVALID_PARAM_RET("OSPF only support 32 networks");
            }
            /* liwh end */

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            masklen = sal_atoi(argv[1]);
            PM_CFG_GET_UINT(areaid, argv[2], para->p_rs);

            /* modified by wangjj for fix bug 43713, not syn with quagga, 2017-04-25 */
            #ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif

            ospf_net_key.family = AF_INET;
            ospf_net_key.prefixlen = masklen;
            sal_memcpy(&ospf_net_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_ospf_network = tbl_ospf_network_get_ospf_network(&ospf_net_key);
            if (NULL == p_db_ospf_network)
            {
                CTCLIB_SLIST_LOOP_DEL(p_master->ospf_network_list, p_ospf_network, listnode, next)
                {
                    if (sal_prefix_match((prefix_t *)&p_ospf_network->key, (prefix_t *)&ospf_net_key) 
                        && p_ospf_network->key.prefixlen == ospf_net_key.prefixlen)
                    {
                        return rc;
                    }                    
                }

                ospf_net.area_id = areaid;
                sal_memcpy(&ospf_net.key, &ospf_net_key, sizeof(ospf_net_key));
                tbl_ospf_network_add_ospf_network(&ospf_net);
                        
                sal_memset(cmd_str, 0, sizeof(cmd_str));
                sal_sprintf(cmd_str, "network %s/%s area %s ", argv[0], argv[1], argv[2]);
                rt_vty_router_ospf_cmd(cmd_str);
            }
            else
            {
                CFG_INVALID_PARAM_RET("There is already the same network statement");
            }
        }
        break;

    case CDB_OPER_DEL:
        if (3 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            masklen = sal_atoi(argv[1]);
            PM_CFG_GET_UINT(areaid, argv[2], para->p_rs);

            /* modified by wangjj for fix bug 43713, not syn with quagga, 2017-04-25 */
            #ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif

            ospf_net_key.family = AF_INET;
            ospf_net_key.prefixlen = masklen;
            sal_memcpy(&ospf_net_key.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_ospf_network = tbl_ospf_network_get_ospf_network(&ospf_net_key);
            if (NULL != p_db_ospf_network && areaid == p_db_ospf_network->area_id)
            {
                tbl_ospf_network_del_ospf_network(&ospf_net_key);
                            
                sal_memset(cmd_str, 0, sizeof(cmd_str));
                sal_sprintf(cmd_str, "no network %s/%s area %s ", argv[0], argv[1], argv[2]);
                rt_vty_router_ospf_cmd(cmd_str);
            }
            /* modified by liwh for bug 47369, 2018-05-23 */
            else if (NULL != p_db_ospf_network && areaid != p_db_ospf_network->area_id)
            {
                CFG_INVALID_PARAM_RET("Area ID not match");
            }
            /* liwh end */
            /* modified by liwh for bug 46493, 2018-05-21 */
            else
            {
                CFG_INVALID_PARAM_RET("Can't find specified network area configuration");    
            }
            /* liwh end */
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_ospf_network(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_ospf_area_range(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_ospf_area_range_master_t *p_master = tbl_ospf_area_range_get_master();
    uint32      mask = 0;
    cdb_node_t *p_node              = NULL;
    tbl_ospf_area_range_t              *p_db_ospf_area_range = NULL;
    tbl_ospf_area_range_t              ospf_area_range;
    tbl_ospf_area_range_key_t          ospf_area_range_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
    int32       retv    = 0;
    uint8       masklen = 0;
    uint32      areaid  = 0;
    char        buf[64];
    addr_ipv4_t ip_prefix;
    uint32 is_advertise = TRUE;

    char        ip_addr_zero[16] = "0.0.0.0";

    char cmd_str[128];

    CFG_DBG_FUNC();

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&ospf_area_range_key, 0, sizeof(ospf_area_range_key));
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf_area_range, 0, sizeof(ospf_area_range));
    p_node = cdb_get_tbl(TBL_OSPF_NETWORK);

    tbl_ospf_t                      *p_db_ospf = NULL;
    tbl_ospf_t                      ospf;
    tbl_ospf_key_t                  ospf_key;
    
    sal_memset(&ospf, 0, sizeof(ospf));
    sal_memset(&ospf_key, 0, sizeof(ospf_key));

    ospf_key.id = OSPF_DEFAULT_INSTANCE;
    p_db_ospf = tbl_ospf_get_ospf(&ospf_key);
    if (NULL == p_db_ospf)
    {
        rc = tbl_ospf_add_ospf(&ospf);
    }

    if (NULL == p_master)
    {
        return rc;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (3 == argc ||4 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

#if 0
            /* modified by liwh for bug 44506, 2017-06-28 */
            if (p_master->ospf_area_range_list->count >= GLB_OSPF_NETWORK_MAX)//??
            {
                CFG_INVALID_PARAM_RET("OSPF only support 32 networks");
            }
            /* liwh end */
#endif
            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            masklen = sal_atoi(argv[1]);
            PM_CFG_GET_UINT(areaid, argv[2], para->p_rs);

            if (0 == sal_memcmp(argv[0], ip_addr_zero, sal_strlen (ip_addr_zero)) || 0 == masklen)
            {
                CFG_INVALID_PARAM_RET("Can not set this range/summary-address that represents default route");
            }
            
            if (4 == argc)
            {
                if (0 == sal_strcmp(argv[3], "advertise"))
                {
                    is_advertise = TRUE;
                }
                else if (0 == sal_strcmp(argv[3], "not-advertise"))
                {
                    is_advertise = FALSE;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid parameters");
                }
                
            }
        
            /* modified by wangjj for fix bug 43713, not syn with quagga, 2017-04-25 */
            #ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif

            /* modified by liwh for bug 51296, 2019-03-18 */
            if (0 == ip_prefix.s_addr)
            {
                CFG_INVALID_PARAM_RET("Can not set this range/summary-address that represents default route");
            }
            /* liwh end */

            ospf_area_range_key.area_id = areaid;
            ospf_area_range_key.p.family = AF_INET;
            ospf_area_range_key.p.prefixlen = masklen;
            sal_memcpy(&ospf_area_range_key.p.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_ospf_area_range = tbl_ospf_area_range_get_ospf_area_range(&ospf_area_range_key);
            if (NULL == p_db_ospf_area_range)
            {
            #if 0
                CTCLIB_SLIST_LOOP_DEL(p_master->ospf_area_range_list, p_ospf_area_range, listnode, next)
                {
                    if (sal_prefix_match((prefix_t *)&p_ospf_area_range->key, (prefix_t *)&ospf_area_range_key) 
                        && p_ospf_area_range->key.prefixlen == ospf_area_range_key.prefixlen)
                    {
                        return rc;
                    }                    
                }
            
                ospf_area_range.area_id = areaid;
                #endif
                sal_memcpy(&ospf_area_range.key, &ospf_area_range_key, sizeof(ospf_area_range_key));
                ospf_area_range.advertise = is_advertise;

                if (_rt_cmd_ospf_area_range_count_max_check())
                {
                    CFG_INVALID_PARAM_RET("The area range num is more than maximum %d!", _rt_cmd_ospf_area_range_count_get_max());
                }
                tbl_ospf_area_range_add_ospf_area_range(&ospf_area_range);
            }
            else
            {
                if (areaid != p_db_ospf_area_range->key.area_id)
                {
                    CFG_INVALID_PARAM_RET("The same range is already configured for other area");
                }

                /*fix bug45545 by chenc*/
                if (is_advertise != p_db_ospf_area_range->advertise)
                {
                    p_db_ospf_area_range->advertise = is_advertise;
                    tbl_ospf_area_range_set_ospf_area_range_field(p_db_ospf_area_range, TBL_OSPF_AREA_RANGE_FLD_ADVERTISE);
                }
            }
        }

        
        sal_memset(cmd_str, 0, sizeof(cmd_str));
        if (argc == 4)
        {
            sal_sprintf(cmd_str, "area %u range %s/%s %s", areaid, cdb_addr_ipv4_val2str(buf, 16, &ip_prefix), argv[1],  argv[3]);
        }
        else
        {
            sal_sprintf(cmd_str, "area %u range %s/%s", areaid, cdb_addr_ipv4_val2str(buf, 16, &ip_prefix), argv[1]);
        }
        rt_vty_router_ospf_cmd(cmd_str);
        break;

    case CDB_OPER_DEL:
        if (3 == argc)
        {
            retv = sal_inet_pton(AF_INET, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv4_t));
            masklen = sal_atoi(argv[1]);
            PM_CFG_GET_UINT(areaid, argv[2], para->p_rs);

            /* modified by wangjj for fix bug 43713, not syn with quagga, 2017-04-25 */
            #ifdef _CENTEC_
            IPV4_LEN_TO_MASK(mask, masklen);
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            ip_prefix.s_addr = ip_prefix.s_addr & mask;
            ip_prefix.s_addr = sal_htonl(ip_prefix.s_addr);
            #endif

            ospf_area_range_key.area_id = areaid;
            ospf_area_range_key.p.family = AF_INET;
            ospf_area_range_key.p.prefixlen = masklen;
            sal_memcpy(&ospf_area_range_key.p.u.prefix4, &ip_prefix, sizeof(addr_ipv4_t));
            p_db_ospf_area_range = tbl_ospf_area_range_get_ospf_area_range(&ospf_area_range_key);
            //if (NULL != p_db_ospf_area_range && areaid == p_db_ospf_area_range->area_id)
            if (NULL != p_db_ospf_area_range)// && areaid == p_db_ospf_area_range->area_id)
            {
                tbl_ospf_area_range_del_ospf_area_range(&ospf_area_range_key);
            }
        }

        sal_memset(cmd_str, 0, sizeof(cmd_str));
        sal_sprintf(cmd_str, "no area %u range %s/%s ", areaid, cdb_addr_ipv4_val2str(buf, 16, &ip_prefix), argv[1]);
        rt_vty_router_ospf_cmd(cmd_str);
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_ospf_area_range(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_ospf6_area_range(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_ospf6_area_range_master_t *p_master = tbl_ospf6_area_range_get_master();
    uint32      mask = 0;
    cdb_node_t *p_node              = NULL;
    tbl_ospf6_area_range_t              *p_db_ospf6_area_range = NULL;
    tbl_ospf6_area_range_t              ospf6_area_range;
    tbl_ospf6_area_range_key_t          ospf6_area_range_key;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
    int32       retv    = 0;
    uint8       masklen = 0;
    uint32      areaid  = 0;
    char        buf[64];
    addr_ipv6_t ip_prefix;
    uint32 is_advertise = TRUE;

    char cmd_str[128];

    CFG_DBG_FUNC();

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(&ip_prefix, 0, sizeof(ip_prefix));
    sal_memset(&ospf6_area_range_key, 0, sizeof(ospf6_area_range_key));
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf6_area_range, 0, sizeof(ospf6_area_range));
    p_node = cdb_get_tbl(TBL_OSPF6_AREA_RANGE);

    tbl_ospf6_t                      *p_db_ospf6 = NULL;
    tbl_ospf6_t                      ospf6;
    tbl_ospf6_key_t                  ospf6_key;
    
    sal_memset(&ospf6, 0, sizeof(ospf6));
    sal_memset(&ospf6_key, 0, sizeof(ospf6_key));

    ospf6_key.id = OSPF_DEFAULT_INSTANCE;
    p_db_ospf6 = tbl_ospf6_get_ospf6(&ospf6_key);
    if (NULL == p_db_ospf6)
    {
        rc = tbl_ospf6_add_ospf6(&ospf6);
    }

    if (NULL == p_master)
    {
        return rc;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (3 == argc ||4 == argc)
        {
            retv = sal_inet_pton(AF_INET6, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv6_t));
            masklen = sal_atoi(argv[1]);
            PM_CFG_GET_UINT(areaid, argv[2], para->p_rs);
            
            if (4 == argc)
            {
                if (0 == sal_strcmp(argv[3], "advertise"))
                {
                    is_advertise = TRUE;
                }
                else if (0 == sal_strcmp(argv[3], "not-advertise"))
                {
                    is_advertise = FALSE;
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid parameters");
                }
                
            }
        
            ospf6_area_range_key.area_id = areaid;
            ospf6_area_range_key.p.family = AF_INET6;
            ospf6_area_range_key.p.prefixlen = masklen;
            sal_memcpy(&ospf6_area_range_key.p.u.prefix6, &ip_prefix, sizeof(addr_ipv6_t));
            p_db_ospf6_area_range = tbl_ospf6_area_range_get_ospf6_area_range(&ospf6_area_range_key);
            if (NULL == p_db_ospf6_area_range)
            {
                sal_memcpy(&ospf6_area_range.key, &ospf6_area_range_key, sizeof(ospf6_area_range_key));
                ospf6_area_range.advertise = is_advertise;
                
                tbl_ospf6_area_range_add_ospf6_area_range(&ospf6_area_range);
            }
            else
            {
                if (areaid != p_db_ospf6_area_range->key.area_id)
                {
                    CFG_INVALID_PARAM_RET("The same range is already configured for other area");
                }

                /*fix bug45545 by chenc*/
                if (is_advertise != p_db_ospf6_area_range->advertise)
                {
                    p_db_ospf6_area_range->advertise = is_advertise;
                    tbl_ospf6_area_range_set_ospf6_area_range_field(p_db_ospf6_area_range, TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE);
                }
            }
        }
        break;

    case CDB_OPER_DEL:
        if (3 == argc)
        {
            retv = sal_inet_pton(AF_INET6, argv[0], buf);
            if (!retv)
            {
                CFG_INVALID_PARAM_RET("Invalid network prefix");
            }

            sal_memcpy(&ip_prefix, buf, sizeof(addr_ipv6_t));
            masklen = sal_atoi(argv[1]);
            PM_CFG_GET_UINT(areaid, argv[2], para->p_rs);

            ospf6_area_range_key.area_id = areaid;
            ospf6_area_range_key.p.family = AF_INET6;
            ospf6_area_range_key.p.prefixlen = masklen;
            sal_memcpy(&ospf6_area_range_key.p.u.prefix6, &ip_prefix, sizeof(addr_ipv6_t));
            p_db_ospf6_area_range = tbl_ospf6_area_range_get_ospf6_area_range(&ospf6_area_range_key);
            if (NULL != p_db_ospf6_area_range)// && areaid == p_db_ospf_area_range->area_id)
            {
                tbl_ospf6_area_range_del_ospf6_area_range(&ospf6_area_range_key);
            }
        }

        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_ospf_area_range(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
rt_cmd_process_ospf_area_auth(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t *p_node              = NULL;
    tbl_ospf_area_auth_t            *p_db_ospf_area_auth = NULL;
    tbl_ospf_area_auth_t            ospf_auth;
    field_parse_t                   field;
    int32                           rc    = PM_E_NONE;
    uint32 areaid = 0;
    tbl_ospf_area_auth_key_t ospf_area_auth_key;
    char cmd_str[128] = {0};

    CFG_DBG_FUNC();

    sal_memset(&ospf_area_auth_key, 0, sizeof(ospf_area_auth_key));
    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf_auth, 0, sizeof(ospf_auth));
    p_node = cdb_get_tbl(TBL_OSPF_AREA_AUTH);

    tbl_ospf_t                      *p_db_ospf  = NULL;
    tbl_ospf_t                      ospf;
    tbl_ospf_key_t                  ospf_key;
    
    sal_memset(&ospf, 0, sizeof(ospf));
    sal_memset(&ospf_key, 0, sizeof(ospf_key));

    ospf_key.id = OSPF_DEFAULT_INSTANCE;
    p_db_ospf = tbl_ospf_get_ospf(&ospf_key);
    if (NULL == p_db_ospf)
    {
        rc = tbl_ospf_add_ospf(&ospf);
    }

    tbl_ospf_area_auth_master_t *p_maser = NULL;
    p_maser = tbl_ospf_area_auth_get_master();
    if (NULL == p_maser)
    {
        return rc;    
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (3 == argc && 0 == sal_strcmp(argv[0], "authentication"))
        {
            PM_CFG_GET_UINT(areaid, argv[1], para->p_rs);
            ospf_area_auth_key.areaid = areaid;
            p_db_ospf_area_auth = tbl_ospf_area_auth_get_ospf_area_auth(&ospf_area_auth_key);
            if (NULL == p_db_ospf_area_auth)
            {
                if (p_maser->ospf_area_auth_hash->count >= OSPF_AREA_LIMIT_DEFAULT)
                {
                    CFG_FUNC_ERR_RET("The max number of ospf area authentication is %u", OSPF_AREA_LIMIT_DEFAULT);
                }
                sal_memcpy(&ospf_auth.key, &ospf_area_auth_key, sizeof(ospf_area_auth_key));
                if (0 == sal_strcmp(argv[2], "message-digest"))
                {
                    ospf_auth.auth_type = OSPF_AREA_AUTH_TYPE_MD5;
                }
                tbl_ospf_area_auth_add_ospf_area_auth(&ospf_auth);
                        
                sal_memset(cmd_str, 0, sizeof(cmd_str));
                sal_sprintf(cmd_str, "area %u authentication message-digest", areaid);
                rt_vty_router_ospf_cmd(cmd_str);
            }
        }
        break;

    case CDB_OPER_DEL:
        if (2 == argc && 0 == sal_strcmp(argv[0], "authentication"))
        {
            PM_CFG_GET_UINT(areaid, argv[1], para->p_rs);
            ospf_area_auth_key.areaid = areaid;
            p_db_ospf_area_auth = tbl_ospf_area_auth_get_ospf_area_auth(&ospf_area_auth_key);
            if (NULL != p_db_ospf_area_auth)
            {
                tbl_ospf_area_auth_del_ospf_area_auth(&ospf_area_auth_key);
                        
                sal_memset(cmd_str, 0, sizeof(cmd_str));
                sal_sprintf(cmd_str, "no area %u authentication", areaid);
                rt_vty_router_ospf_cmd(cmd_str);
            }
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _rt_cmd_get_ospf_area_auth(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_process_ospf_area_stub(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                      *p_node = NULL;
    tbl_ospf_area_stub_t            *p_db_ospf_area_stub = NULL;
    tbl_ospf_area_stub_t            ospf_stub;
	tbl_ospf_area_nssa_t            ospf_nssa;
    field_parse_t                   field;
    int32                           rc = PM_E_NONE;
    tbl_ospf_area_stub_master_t     *p_maser = NULL;
    uint32 areaid = 0;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint8 flag = 0;

    CFG_DBG_FUNC();

    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf_stub, 0, sizeof(ospf_stub));
    p_node = cdb_get_tbl(TBL_OSPF_AREA_STUB);

    p_maser = tbl_ospf_area_stub_get_master();
    if(NULL == p_maser)
        return rc;    

    switch(para->oper)
    {
        case CDB_OPER_ADD:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf_stub.key.areaid = areaid;
            p_db_ospf_area_stub = tbl_ospf_area_stub_get_ospf_area_stub(&ospf_stub.key);
            if(!p_db_ospf_area_stub)
                tbl_ospf_area_stub_add_ospf_area_stub(&ospf_stub);
           
			/* remove from nssa table if area already configed as a nssa */
            ospf_nssa.key.areaid = areaid;
            tbl_ospf_area_nssa_del_ospf_area_nssa(&ospf_nssa.key);	 
            break;
        case CDB_OPER_DEL:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf_stub.key.areaid = areaid;
            p_db_ospf_area_stub = tbl_ospf_area_stub_get_ospf_area_stub(&ospf_stub.key);
            if(p_db_ospf_area_stub)
                tbl_ospf_area_stub_del_ospf_area_stub(&ospf_stub.key);
            
            break;
        case CDB_OPER_SET:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf_stub.key.areaid = areaid;
            p_db_ospf_area_stub = tbl_ospf_area_stub_get_ospf_area_stub(&ospf_stub.key);
            if(!p_db_ospf_area_stub)
            {
                CFG_CONFLICT_RET("OSPF stub area %d not found", ospf_stub.key.areaid);
            }

            field_id = cdb_get_field_id(argv[1], p_node, &p_field);
            switch(field_id)
            {
                case TBL_OSPF_AREA_STUB_FLD_NO_SUMMARY:
                    PM_CFG_GET_BOOL(flag, argv[2], para->p_rs);
                    ospf_stub.no_summary = flag;
                    rc = tbl_ospf_area_stub_set_ospf_area_stub_field(&ospf_stub, field_id);
                    break;
                    
                default:
                    CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
                    break;
            }
            break;

        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = _rt_cmd_get_ospf_area_stub(para, p_node, &field);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}

int32
rt_cmd_process_ospf6_area_stub(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                      *p_node = NULL;
    tbl_ospf6_area_stub_t            *p_db_ospf6_area_stub = NULL;
    tbl_ospf6_area_stub_t            ospf6_stub;
    field_parse_t                   field;
    int32                           rc = PM_E_NONE;
    tbl_ospf6_area_stub_master_t     *p_maser = NULL;
    uint32 areaid = 0;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint8 flag = 0;

    CFG_DBG_FUNC();

    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf6_stub, 0, sizeof(ospf6_stub));
    p_node = cdb_get_tbl(TBL_OSPF6_AREA_STUB);

    p_maser = tbl_ospf6_area_stub_get_master();
    if(NULL == p_maser)
        return rc;    

    switch(para->oper)
    {
        case CDB_OPER_ADD:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf6_stub.key.areaid = areaid;
            p_db_ospf6_area_stub = tbl_ospf6_area_stub_get_ospf6_area_stub(&ospf6_stub.key);
            if(!p_db_ospf6_area_stub)
                tbl_ospf6_area_stub_add_ospf6_area_stub(&ospf6_stub);
           
            break;
        case CDB_OPER_DEL:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf6_stub.key.areaid = areaid;
            p_db_ospf6_area_stub = tbl_ospf6_area_stub_get_ospf6_area_stub(&ospf6_stub.key);
            if(p_db_ospf6_area_stub)
                tbl_ospf6_area_stub_del_ospf6_area_stub(&ospf6_stub.key);
            
            break;
        case CDB_OPER_SET:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf6_stub.key.areaid = areaid;
            p_db_ospf6_area_stub = tbl_ospf6_area_stub_get_ospf6_area_stub(&ospf6_stub.key);
            if(!p_db_ospf6_area_stub)
            {
                CFG_CONFLICT_RET("OSPF stub area %d not found", ospf6_stub.key.areaid);
            }

            field_id = cdb_get_field_id(argv[1], p_node, &p_field);
            switch(field_id)
            {
                case TBL_OSPF6_AREA_STUB_FLD_NO_SUMMARY:
                    PM_CFG_GET_BOOL(flag, argv[2], para->p_rs);
                    ospf6_stub.no_summary = flag;
                    rc = tbl_ospf6_area_stub_set_ospf6_area_stub_field(&ospf6_stub, field_id);
                    break;
                    
                default:
                    CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
                    break;
            }
            break;

        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            //rc = _rt_cmd_get_ospf6_area_stub(para, p_node, &field);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}



int32
rt_cmd_process_ospf_area_nssa(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                      *p_node = NULL;
    tbl_ospf_area_nssa_t            *p_db_ospf_area_nssa = NULL;
    tbl_ospf_area_nssa_t            ospf_nssa;
	tbl_ospf_area_stub_t            ospf_stub;
    field_parse_t                   field;
    int32                           rc = PM_E_NONE;
    tbl_ospf_area_nssa_master_t     *p_maser = NULL;
    uint32 areaid = 0;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint8 flag = 0;

    CFG_DBG_FUNC();

    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf_nssa, 0, sizeof(ospf_nssa));
    p_node = cdb_get_tbl(TBL_OSPF_AREA_NSSA);

    p_maser = tbl_ospf_area_nssa_get_master();
    if(NULL == p_maser)
        return rc;

    switch(para->oper)
    {
        case CDB_OPER_ADD:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf_nssa.key.areaid = areaid;
            p_db_ospf_area_nssa = tbl_ospf_area_nssa_get_ospf_area_nssa(&ospf_nssa.key);
            if(!p_db_ospf_area_nssa)
                tbl_ospf_area_nssa_add_ospf_area_nssa(&ospf_nssa);
           
			 /* remove from stub table if area already configed as a stub */
            ospf_stub.key.areaid = areaid;
            tbl_ospf_area_stub_del_ospf_area_stub(&ospf_stub.key); 
            break;
        case CDB_OPER_DEL:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf_nssa.key.areaid = areaid;
            p_db_ospf_area_nssa = tbl_ospf_area_nssa_get_ospf_area_nssa(&ospf_nssa.key);
            if(p_db_ospf_area_nssa)
                tbl_ospf_area_nssa_del_ospf_area_nssa(&ospf_nssa.key);
            
            break;
        case CDB_OPER_SET:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            ospf_nssa.key.areaid = areaid;
            p_db_ospf_area_nssa = tbl_ospf_area_nssa_get_ospf_area_nssa(&ospf_nssa.key);
            if(!p_db_ospf_area_nssa)
            {
                CFG_CONFLICT_RET("OSPF nssa area %d not found", ospf_nssa.key.areaid);
            }

            field_id = cdb_get_field_id(argv[1], p_node, &p_field);
            switch(field_id)
            {
                case TBL_OSPF_AREA_NSSA_FLD_NSSA_ROLE:
                    if(sal_strncmp(argv[2], "translate-c", 11) == 0)
                        ospf_nssa.nssa_role = OSPF_NSSA_TRANSLATE_ROLE_CANDIDATE;
                    else if(sal_strncmp(argv[2], "translate-n", 11) == 0)
                        ospf_nssa.nssa_role = OSPF_NSSA_TRANSLATE_ROLE_NEVER;
                    else if(sal_strncmp(argv[2], "translate-a", 11) == 0)
                        ospf_nssa.nssa_role = OSPF_NSSA_TRANSLATE_ROLE_ALWAYS;
                    else
                        ospf_nssa.nssa_role = OSPF_NSSA_TRANSLATE_ROLE_DEFAULT;
                    rc = tbl_ospf_area_nssa_set_ospf_area_nssa_field(&ospf_nssa, field_id);
                    break;

                case TBL_OSPF_AREA_NSSA_FLD_NO_SUMMARY:
                    PM_CFG_GET_BOOL(flag, argv[2], para->p_rs);
                    ospf_nssa.no_summary = flag;
                    rc = tbl_ospf_area_nssa_set_ospf_area_nssa_field(&ospf_nssa, field_id);
                    break;
                    
                default:
                    CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[1]);
                    break;
            }
            break;

        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = _rt_cmd_get_ospf_area_nssa(para, p_node, &field);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}


int32
rt_cmd_process_ospf_area_vlink(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t                      *p_node = NULL;
    tbl_ospf_area_vlink_t           *p_db_ospf_area_vlink = NULL;
    tbl_ospf_area_vlink_t           ospf_vlink;
    field_parse_t                   field;
    int32                           rc = PM_E_NONE;
    tbl_ospf_area_vlink_master_t     *p_maser = NULL;
    uint32 areaid = 0, value = 0;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;

    CFG_DBG_FUNC();

    sal_memset(&field, 0, sizeof(field));
    sal_memset(&ospf_vlink, 0, sizeof(ospf_vlink));
    p_node = cdb_get_tbl(TBL_OSPF_AREA_VLINK);

    p_maser = tbl_ospf_area_vlink_get_master();
    if(NULL == p_maser)
        return rc;

    switch(para->oper)
    {
        case CDB_OPER_ADD:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            CFG_PARSER_IP_ADDR(argv[1], &ospf_vlink.key.vl_peer, para->p_rs);
            ospf_vlink.key.area_id = areaid;
            p_db_ospf_area_vlink = tbl_ospf_area_vlink_get_ospf_area_vlink(&ospf_vlink.key);
            if(!p_db_ospf_area_vlink)
                tbl_ospf_area_vlink_add_ospf_area_vlink(&ospf_vlink);
            
            break;
        case CDB_OPER_DEL:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            CFG_PARSER_IP_ADDR(argv[1], &ospf_vlink.key.vl_peer, para->p_rs);
            ospf_vlink.key.area_id = areaid;
            p_db_ospf_area_vlink = tbl_ospf_area_vlink_get_ospf_area_vlink(&ospf_vlink.key);
            if(p_db_ospf_area_vlink)
                tbl_ospf_area_vlink_del_ospf_area_vlink(&ospf_vlink.key);
            
            break;
        case CDB_OPER_SET:
            PM_CFG_GET_UINT(areaid, argv[0], para->p_rs);
            CFG_PARSER_IP_ADDR(argv[1], &ospf_vlink.key.vl_peer, para->p_rs);
            ospf_vlink.key.area_id = areaid;
            p_db_ospf_area_vlink = tbl_ospf_area_vlink_get_ospf_area_vlink(&ospf_vlink.key);
            if(!p_db_ospf_area_vlink)
            {
                CFG_CONFLICT_RET("OSPF vlink area %d not found", ospf_vlink.key.area_id);
            }

            field_id = cdb_get_field_id(argv[2], p_node, &p_field);
            switch(field_id)
            {
                case TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG:
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    ospf_vlink.auth_flag = value;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE:
                    if(argc == 3)
                        ospf_vlink.auth_type = OSPF_VLINK_AUTH_TYPE_DEFAULT;
                    else if(sal_strncmp(argv[3], "message-digest", 14) == 0)
                        ospf_vlink.auth_type = OSPF_VLINK_AUTH_TYPE_MD5;
                    else if(sal_strncmp(argv[3], "null", 4) == 0)
                        ospf_vlink.auth_type = OSPF_VLINK_AUTH_TYPE_NULL;
                    else
                        ospf_vlink.auth_type = OSPF_VLINK_AUTH_TYPE_DEFAULT;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY:
                    if(argc == 3)
                        sal_strcpy(ospf_vlink.auth_key, "");
                    else
                        sal_strcpy(ospf_vlink.auth_key, argv[3]);
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_MD5_ID:
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    ospf_vlink.md5_id = value;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_MD5_KEY:
                    if(argc == 3)
                        sal_strcpy(ospf_vlink.md5_key, "");
                    else
                        sal_strcpy(ospf_vlink.md5_key, argv[3]);
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL:
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    ospf_vlink.hello_interval = value;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL:
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    ospf_vlink.dead_interval = value;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL:
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    ospf_vlink.retransmit_interval = value;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;
                case TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY:
                    PM_CFG_GET_UINT(value, argv[3], para->p_rs);
                    ospf_vlink.transmit_delay = value;
                    rc = tbl_ospf_area_vlink_set_ospf_area_vlink_field(&ospf_vlink, field_id);
                    break;

                default:
                    CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[2]);
                    break;
            }
            break;

        case CDB_OPER_GET:
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
            rc = _rt_cmd_get_ospf_area_vlink(para, p_node, &field);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid action");
            break;
    }

    return rc;
}

int32
rt_cmd_show_ip_if_brief(tbl_route_if_t *p_db_rtif, tbl_interface_t *p_db_if, FILE *fp)
{
    int32               rc  = PM_E_NONE;
    ds_connected_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *rt_listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];
    uint32              has_addr = 0;

    char buf[CMD_BUF_256];
    char buf2[CMD_BUF_256];

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));

    /* Interface name. */
    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
    sal_fprintf(fp, "%-20s  ", ifname_ext);

    /* IP addresss, show only primary. */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
    {
        if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
        {
            if (0x0 != p_db_conn->key.address.u.prefix4.s_addr)
            {
                sal_fprintf(fp, "%-15s ",
                    cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_db_conn->key.address.u.prefix4));
            }

            has_addr = 1;
        }
    }

    if (!has_addr)
    {
        sal_fprintf(fp, "unassigned      ");
    }

    /* Status. */
    if (tbl_interface_is_running(p_db_if))
    {
        sal_fprintf(fp, "%-24s", "up");
    }
    else if (tbl_interface_is_up(p_db_if))
    {
        sal_fprintf(fp, "%-24s", "down");
    }
    else
    {
        sal_fprintf(fp, "%-24s", "administratively down");
    }

    /* Protocol. */
    if (tbl_interface_is_running(p_db_if))
        sal_fprintf(fp, "up");
    else
        sal_fprintf(fp, "down");

    sal_fprintf(fp, "\n");

    return rc;
}

int32
rt_cmd_show_route_interface(void *obj, tbl_iter_args_t *pargs)
{
    FILE                *fp = (FILE *)pargs->argv[0];
    char                *name = (char *)pargs->argv[1];
    tbl_route_if_t      *p_db_rtif = obj;
    tbl_interface_t     *p_db_if = NULL;
    int32               rc  = PM_E_NONE;

    if (NULL == p_db_rtif)
    {
        return rc;
    }

    if (name)
    {
        if (0 != strcmp(name, p_db_rtif->key.name))
        {
            return rc;
        }
    }

    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (NULL == p_db_if)
    {
        return rc;
    }

    if ((GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_db_if))))
    {
        rc = rt_cmd_show_ip_if_brief(p_db_rtif, p_db_if, fp);
    }

    return rc;
}

int32
rt_cmd_show_ip_if_entry(char *if_name, cfg_cmd_para_t *para)
{
    FILE            *fp = NULL;
    int32           rc  = PM_E_NONE;
    tbl_iter_args_t     args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "Interface             IP-Address      Status                  Protocol\n");
    sal_fprintf(fp, "---------------------+---------------+-----------------------+--------\n");

    args.argv[0] = fp;
    args.argv[1] = if_name;
    rc = tbl_route_if_iterate(rt_cmd_show_route_interface, &args);

    sal_fclose(fp);

    return rc;
}


int32
rt_cmd_show_ip_if_detail(tbl_route_if_t *p_db_rtif, tbl_interface_t *p_db_if, FILE *fp)
{
    int32               rc  = PM_E_NONE;
    ds_connected_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *rt_listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];

    char buf[CMD_BUF_256];
    char buf2[CMD_BUF_256];

    int32 is_ipaddress_title_displayed = FALSE;

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));

    /* Interface name. */
    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
    sal_fprintf(fp, "Interface %s\n", ifname_ext);

    /* Status. */
    if (tbl_interface_is_running (p_db_if))
        sal_fprintf(fp, "  Interface current state: UP\n");
    else if (tbl_interface_is_up(p_db_if))
        sal_fprintf(fp, "  Interface current state: DOWN\n");
    else
        sal_fprintf(fp, "  Interface current state: administratively DOWN\n");

    if ((GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_db_if))))
    {
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
        {
            if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
            {
                if ((NULL != p_db_rtif) && (0x0 != p_db_conn->key.address.u.prefix4.s_addr))
                {
                    if (FALSE == is_ipaddress_title_displayed)
                    {
                        /* IP addresss show*/
                        sal_fprintf(fp, "  Internet address(es):\n");
                        is_ipaddress_title_displayed = TRUE;
                    }
                    
                    sal_fprintf(fp, CMD_MODE_INDENT"  %s/%d",
                        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_db_conn->key.address.u.prefix4), p_db_conn->key.address.prefixlen);
                    sal_fprintf(fp, CMD_MODE_INDENT"broadcast %s/%d\n",
                        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_db_conn->destination.u.prefix4), p_db_conn->destination.prefixlen);
                }
            }
        }

        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, rt_listnode)
        {
            if (GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
            {
                if ((NULL != p_db_rtif) && (0x0 != p_db_conn->key.address.u.prefix4.s_addr))
                {
                    if (FALSE == is_ipaddress_title_displayed)
                    {
                        /* IP addresss show*/
                        sal_fprintf(fp, "  Internet address(es):\n");
                        is_ipaddress_title_displayed = TRUE;
                    }
                    
                    sal_fprintf(fp, CMD_MODE_INDENT"  %s/%d",
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_db_conn->key.address.u.prefix4), p_db_conn->key.address.prefixlen);
                    sal_fprintf(fp, CMD_MODE_INDENT"broadcast %s/%d secondary\n",
                        cdb_addr_ipv4_val2str(buf2, CMD_BUF_256, &p_db_conn->destination.u.prefix4), p_db_conn->destination.prefixlen);
                }
            }
        }
    }

    /*fix bug 45238 , when IP address is not configured, l3 msg should not display*/
    if (FALSE == is_ipaddress_title_displayed)
    {
        return rc;
    }
    
    sal_fprintf(fp, "  The maximum transmit unit is %d bytes\n", p_db_rtif->mtu);

    /*fix bug 45232, The icmp status should display when show ip interface*/
    /*ICMP information*/
    sal_fprintf(fp, "  ICMP redirects are %s sent\n", p_db_rtif->ip_redirects_en ? "always" : "never");
                        
    /*Arp information*/
    sal_fprintf(fp, "  ARP timeout %02d:%02d:%02d,",
        p_db_rtif->arp_timeout/(60*60), (p_db_rtif->arp_timeout%(60*60))/60, ((p_db_rtif->arp_timeout%60)));
    sal_fprintf(fp, "  ARP retry interval %ds\n",
        p_db_rtif->arp_retrans_time);

    sal_fprintf(fp, "  ARP Proxy is %s, Local ARP Proxy is %s\n", (p_db_rtif->arp_proxy_en ? "enabled" : "disabled"),
              (p_db_rtif->local_arp_proxy_en ? "enabled" : "disabled"));

    if (p_db_rtif->unicast_rpf_en)
    {
        sal_fprintf(fp, "  IP verify source reachable-via RX, is enabled\n");
    }

    return rc;
}

int32
route_cmd_process_show_if_brief(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    addr_ipv4_t         ip_addr;
    char                ifname[IFNAME_SIZE];
    tbl_route_if_key_t  rtif_key;
    tbl_interface_t     *p_db_if    = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    FILE                *fp = NULL;
    char                 ifname_ext[IFNAME_EXT_SIZE];

    CFG_DBG_FUNC();
    sal_memset(&ip_addr, 0, sizeof(addr_ipv4_t));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memset(&rtif_key, 0, sizeof(rtif_key));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Num of parameters configured is invalid!");
        }
        else if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "all"))
            {
                if (NULL != p_master)
                {
                    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
                    fp = sal_fopen(para->p_show->path, "w+");
                    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
                    {
                        p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
                        if (NULL == p_db_if)
                        {
                            continue;
                        }

                        rt_cmd_show_ip_if_detail(p_db_rtif, p_db_if, fp);
                    }

                    sal_fclose(fp);
                }

                return PM_E_NONE;
            }
            else
            {
                sal_strncpy(ifname, argv[0], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface_by_name(ifname);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("Please check interface name configuration!");
                }

                sal_memcpy(rtif_key.name, ifname, sizeof(ifname));
                p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
                if (NULL == p_db_rtif)
                {
                    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s not configured for L3 interface!", ifname_ext);
                }

                CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
                fp = sal_fopen(para->p_show->path, "w+");
                rt_cmd_show_ip_if_detail(p_db_rtif, p_db_if, fp);
                sal_fclose(fp);
            }
        }
        else if (2 == argc)
        {
            if (0 == sal_strcmp(argv[1], "brief"))
            {
                if (0 == sal_strcmp(argv[0], "all"))
                {
                    rt_cmd_show_ip_if_entry(NULL, para);
                }
                else
                {
                    sal_strncpy(ifname, argv[0], IFNAME_SIZE);
                    p_db_if = tbl_interface_get_interface_by_name(ifname);
                    if (NULL == p_db_if)
                    {
                        CFG_INVALID_PARAM_RET("Please check interface name configuration!");
                    }

                    sal_memcpy(rtif_key.name, ifname, sizeof(ifname));
                    p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
                    if (NULL == p_db_rtif)
                    {
                        IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s not configured for L3 interface!", ifname_ext);
                    }

                    rt_cmd_show_ip_if_entry(ifname, para);

                }
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support update action");
        break;
    }

    return PM_E_NONE;
}


int32
rt_cmd_show_ipv6_if_brief(tbl_route_if_t *p_db_rtif, tbl_interface_t *p_db_if, FILE *fp)
{
    int32               rc  = PM_E_NONE;
    ds_connected_v6_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *rt_listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];
    uint32              has_addr = 0;

    char buf[CMD_BUF_256];
    char buf2[CMD_BUF_256];

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));

    /* Interface name. */
    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
    sal_fprintf(fp, "%-11s ", ifname_ext);

    /* Status. */
    if (tbl_interface_is_running(p_db_if))
    {
        sal_fprintf(fp, "%-12s ", "up");
    }
    else if (tbl_interface_is_up(p_db_if))
    {
        sal_fprintf(fp, "%-12s ", "down");
    }
    else
    {
        sal_fprintf(fp, "%-12s ", "admin down");
    }

    /* Protocol. */
    if (tbl_interface_is_running(p_db_if))
        sal_fprintf(fp, "%-8s ","up");
    else
        sal_fprintf(fp, "%-8s ","down");

    /* IP addresss, show only primary. */
    CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn, rt_listnode)
    {
        if (!has_addr)
        {
            sal_fprintf(fp, "%s\n",
                cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_db_conn->key.address.u.prefix6));
            has_addr = 1;
        }
        else
        {
            sal_fprintf(fp, "%-34s%s\n", " ",
                cdb_addr_ipv6_val2str(buf2, CMD_BUF_256, &p_db_conn->key.address.u.prefix6));
        }
    }

    if (!has_addr)
    {
        sal_fprintf(fp, "unassigned      ");
    }

    sal_fprintf(fp, "\n");

    return rc;
}

int32
rt_cmd_show_route_interface_v6(void *obj, tbl_iter_args_t *pargs)
{
    FILE                *fp = (FILE *)pargs->argv[0];
    char                *name = (char *)pargs->argv[1];
    tbl_route_if_t      *p_db_rtif = obj;
    tbl_interface_t     *p_db_if = NULL;
    int32               rc  = PM_E_NONE;

    if (NULL == p_db_rtif)
    {
        return rc;
    }

    if (name)
    {
        if (0 != sal_strcmp(name, p_db_rtif->key.name))
        {
            return rc;
        }
    }

    p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
    if (NULL == p_db_if)
    {
        return rc;
    }

    if ((GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_db_if))))
    {
        rc = rt_cmd_show_ipv6_if_brief(p_db_rtif, p_db_if, fp);
    }

    return rc;
}

int32
rt_cmd_show_ipv6_if_entry(char *if_name, cfg_cmd_para_t *para)
{
    FILE            *fp = NULL;
    int32           rc  = PM_E_NONE;
    tbl_iter_args_t     args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    //sal_fprintf(fp, "%-20s%-24s%10s  %s", "Interface", "Status", "Protocol", "IPv6 Address\n");
    sal_fprintf(fp, "%-11s %-12s %8s %s", "Interface", "Status", "Protocol", "IPv6 Address\n");
    sal_fprintf(fp, "-----------+------------+--------+----------------------------------------\n");

    args.argv[0] = fp;
    args.argv[1] = if_name;
    rc = tbl_route_if_iterate(rt_cmd_show_route_interface_v6, &args);

    sal_fclose(fp);

    return rc;
}

int32
prefix_cli_out (FILE *fp, prefix_t *p)
{
  char str[INET6_ADDRSTRLEN] = {0};

  prefix_val2str(str, INET6_ADDRSTRLEN, p, FALSE);
  sal_fprintf(fp, "%s", str);
  return sal_strlen (str);
}

void
connected_dump_vty (FILE *fp, ds_connected_v6_t *connected)
{
  prefix_t *p = NULL;

  prefix_t prefix = {0};

  /* Set interface pointer. */

  /* Print interface address. */
  p = &connected->key.address;

  if (p->family == AF_INET6)
  {
    prefix_cli_out(fp, p);
    if (!IN6_IS_ADDR_LINKLOCAL(&(connected->key.address.u.prefix6)))
    {
      ip_prefix_copy(&prefix, &connected->key.address);
      ip_apply_mask(&prefix);
      p = &prefix;
      sal_fprintf(fp, ", subnet is ");
      prefix_cli_out(fp, p);
      sal_fprintf(fp, "/%d", p->prefixlen);
      if (GLB_FLAG_ISSET(connected->flags, RT_IF_ADDRV6_GLOBAL_EUI64))
      {
        sal_fprintf(fp, " [EUI");
      }
    }

    /* If there is destination address, print it. */
    p = &connected->destination;
    if (p)
      {
      #if 0
        if (p->family == AF_INET)
  	if (ifp->flags & IFF_BROADCAST)
  	  {
  	    sal_fprintf(fp, " broadcast ");
  	    prefix_cli_out (fp, p);
  	  }

        if (ifp->flags & IFF_POINTOPOINT)
  	{
  	  sal_fprintf(fp, " pointopoint ");
  	  prefix_cli_out (fp, p);
  	}
  	#endif
      }

    if (GLB_FLAG_ISSET (connected->flags, RT_IF_ADDR_SECONDARY))
      sal_fprintf(fp, " secondary");

    if (GLB_FLAG_ISSET (connected->flags, RT_IF_ADDR_ANYCAST))
      sal_fprintf(fp, " anycast");

    if (GLB_FLAG_ISSET (connected->flags, RT_IF_ADDR_VIRTUAL))
      sal_fprintf(fp, " virtual");

    //if (connected->label)
    //  sal_fprintf(fp, " %s", connected->label);

    if (!GLB_FLAG_ISSET(connected->flags, RT_IF_ADDRV6_GLOBAL_EUI64))
    {
      if (GLB_FLAG_ISSET(connected->flags, RT_IF_ADDR_DUPLICATE))
        sal_fprintf(fp, " %s", "[DUP]");
      else if (!GLB_FLAG_ISSET(connected->flags, RT_IF_ADDRV6_REAL))
        sal_fprintf(fp, " %s", "[TEN]");
    }
    else
    {
      if (GLB_FLAG_ISSET(connected->flags, RT_IF_ADDR_DUPLICATE))
        sal_fprintf(fp, "%s", "/DUP]");
      else if (!GLB_FLAG_ISSET(connected->flags, RT_IF_ADDRV6_REAL))
        sal_fprintf(fp, "%s", "/TEN]");
      else
        sal_fprintf(fp, "]");
    }
  }
  sal_fprintf(fp, "\n");
}

int32
rt_cmd_show_ipv6_if_detail(tbl_route_if_t *p_db_rtif, tbl_interface_t *p_db_if, FILE *fp)
{
    int32               rc  = PM_E_NONE;
    ds_connected_v6_t      *p_db_conn = NULL;
    ctclib_slistnode_t  *rt_listnode = NULL;
    char                ifname_ext[IFNAME_EXT_SIZE];
    tbl_route_global_t *p_route_global_master = tbl_route_global_get_route_global();

    char buf[CMD_BUF_256];
    char buf2[CMD_BUF_256];

    int32 is_ipaddress_title_displayed = FALSE;

    sal_memset(buf, 0, sizeof(buf));
    sal_memset(buf2, 0, sizeof(buf2));
    sal_memset(ifname_ext, 0, sizeof(ifname_ext));

    /* Interface name. */
    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
    sal_fprintf(fp, "Interface %s\n", ifname_ext);

    /* Status. */
    if (tbl_interface_is_running (p_db_if))
        sal_fprintf(fp, "  Interface current state: UP\n");
    else if (tbl_interface_is_up(p_db_if))
        sal_fprintf(fp, "  Interface current state: DOWN\n");
    else
        sal_fprintf(fp, "  Interface current state: administratively DOWN\n");

    if (0 == p_db_rtif->ifc_ipv6.obj_list->count)
    {
        return PM_E_NONE;
    }

    if (NULL != p_route_global_master && FALSE == p_route_global_master->ipv6_enable)
    {
        return PM_E_NONE;
    }

    sal_fprintf(fp, "  The maximum transmit unit is %d bytes\n", p_db_rtif->mtu);
    
    if ((GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_LOOPBACK_IF == p_db_if->hw_type)
            || (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type && (!INTERFACE_IS_LAG_MEMBER(p_db_if))))
    {
        /*link local*/
        CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn, rt_listnode)
        {
            if (IN6_IS_ADDR_LINKLOCAL(&p_db_conn->key.address.u.prefix6))
            {
                if ((NULL != p_db_rtif))
                {
                    if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_DUPLICATE))
                    {
                        sal_fprintf(fp, "  IPv6 is enabled, link-local address is ");
                    }
                    else
                    {
                        sal_fprintf(fp, "  IPv6 is disabled, link-local address is ");
                    }

                    connected_dump_vty(fp, p_db_conn);
                    is_ipaddress_title_displayed = TRUE;
                }
            }
        }

        /*global unicast address*/
        if (p_db_rtif->ifc_ipv6.obj_list->count > 1)
        {
            sal_fprintf(fp, "  Global unicast address(es):\n");
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv6.obj_list, p_db_conn, rt_listnode)
            {
                if (!IN6_IS_ADDR_LINKLOCAL(&p_db_conn->key.address.u.prefix6))
                {
                    sal_fprintf(fp, "    ");
                    connected_dump_vty(fp, p_db_conn);
                    is_ipaddress_title_displayed = TRUE;
                }
            }
        }
        else
        {
            sal_fprintf(fp, "  No global unicast address is configured\n");
        }
    }

    /*fix bug 45238 , when IP address is not configured, l3 msg should not display*/
    if (FALSE == is_ipaddress_title_displayed)
    {
        return rc;
    }
    
    /*fix bug 45232, The icmp status should display when show ip interface*/
    /*ICMP information*/
    //sal_fprintf(fp, "  ICMP redirects are %s sent\n", p_db_rtif->ip_redirects_en ? "always" : "never");
    sal_fprintf(fp, "  ICMP redirects are %s sent\n", "always" );

    /* support nd modified by liwh for bug 47547, 2018-07-17 */
    nd_api_if_show(fp, p_db_rtif);
    /* liwh end */
   
    if (p_db_rtif->unicast_rpf_en_v6)
    {
        sal_fprintf(fp, "  IPv6 verify source reachable-via RX, is enabled\n");
    }

    return rc;
}

int32
route_cmd_process_show_v6_if_brief(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    addr_ipv6_t         ipv6_addr;
    char                ifname[IFNAME_SIZE];
    tbl_route_if_key_t  rtif_key;
    tbl_interface_t     *p_db_if    = NULL;
    tbl_route_if_t      *p_db_rtif  = NULL;
    ctclib_slistnode_t  *listnode   = NULL;
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    FILE                *fp = NULL;
    char                 ifname_ext[IFNAME_EXT_SIZE];

    CFG_DBG_FUNC();
    sal_memset(&ipv6_addr, 0, sizeof(addr_ipv6_t));
    sal_memset(ifname, 0, sizeof(ifname));
    sal_memset(&rtif_key, 0, sizeof(rtif_key));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (FALSE == rt_api_is_ipv6_profile())
        {
            CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
        }
            
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Num of parameters configured is invalid!");
        }
        else if (1 == argc)
        {
            if (0 == sal_strcmp(argv[0], "all"))
            {
                if (NULL != p_master)
                {
                    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
                    fp = sal_fopen(para->p_show->path, "w+");
                    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
                    {
                        p_db_if = tbl_interface_get_interface_by_name(p_db_rtif->key.name);
                        if (NULL == p_db_if)
                        {
                            continue;
                        }

                        rt_cmd_show_ipv6_if_detail(p_db_rtif, p_db_if, fp);
                    }

                    sal_fclose(fp);
                }

                return PM_E_NONE;
            }
            else
            {
                sal_strncpy(ifname, argv[0], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface_by_name(ifname);
                if (NULL == p_db_if)
                {
                    CFG_INVALID_PARAM_RET("No such interface");
                }

                sal_memcpy(rtif_key.name, ifname, sizeof(ifname));
                p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
                if (NULL == p_db_rtif)
                {
                    IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s not configured for L3 interface!", ifname_ext);
                }

                CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
                fp = sal_fopen(para->p_show->path, "w+");
                rt_cmd_show_ipv6_if_detail(p_db_rtif, p_db_if, fp);
                sal_fclose(fp);
            }
        }
        else if (2 == argc)
        {
            if (0 == sal_strcmp(argv[1], "brief"))
            {
                if (0 == sal_strcmp(argv[0], "all"))
                {
                    rt_cmd_show_ipv6_if_entry(NULL, para);
                }
                else
                {
                    sal_strncpy(ifname, argv[0], IFNAME_SIZE);
                    p_db_if = tbl_interface_get_interface_by_name(ifname);
                    if (NULL == p_db_if)
                    {
                        CFG_INVALID_PARAM_RET("No such interface");
                    }

                    sal_memcpy(rtif_key.name, ifname, sizeof(ifname));
                    p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
                    if (NULL == p_db_rtif)
                    {
                        IFNAME_ETH2SHORT(p_db_if->key.name, ifname_ext);
                        CFG_INVALID_PARAM_RET("Interface %s not configured for L3 interface!", ifname_ext);
                    }

                    rt_cmd_show_ipv6_if_entry(ifname, para);

                }
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Only support get action");
        break;
    }

    return PM_E_NONE;
}

int32
route_clear_ipv6_route_static_all()
{
    tbl_static_route_cfg_t  static_route;
    int32 rc = PM_E_NONE;
    tbl_static_route_cfg_master_t *p_master = tbl_static_route_cfg_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_static_route_cfg_t *p_db_rst_cfg = NULL;
    prefix_t gate_tmp = {0};
    uint32 distance = 0;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->rst_cfg_list, p_db_rst_cfg, listnode, next)
    {
        if (NULL == p_db_rst_cfg)
        {
            continue;
        }

        if (AF_INET6 != p_db_rst_cfg->key.p.family)
        {
            continue;
        }

        sal_memset(&static_route, 0, sizeof(static_route));
        sal_memcpy(&static_route, p_db_rst_cfg, sizeof(static_route));

        distance = p_db_rst_cfg->distance;
                
        /*delete tbl*/
        rc = tbl_static_route_cfg_del_static_route_cfg(&p_db_rst_cfg->key);
        
        sal_memset(&gate_tmp, 0, sizeof(gate_tmp));
        gate_tmp.family = static_route.key.p.family;
        sal_memcpy(&gate_tmp.u.prefix4, &static_route.key.nh_addr.ipv4, sizeof(addr_ipv4_t));
        sal_memcpy(&gate_tmp.u.prefix6, &static_route.key.nh_addr.ipv6, sizeof(addr_ipv6_t));

        rt_cmd_static_rt_cnt_del(0, &static_route.key.p, static_route.key.p.prefixlen, distance, FALSE, &gate_tmp, FALSE);

        // for clear all, add ifindex
        rc = rt_cmd_del_static_route_by_syscmd(&static_route.key.p, &gate_tmp, static_route.key.p.prefixlen, static_route.key.ifindex_v6, FALSE);
    }
    
    return rc;
}

static int
_route_api_show_ip_route_add_fib_fail_interator(void *obj, tbl_iter_args_t *pargs)
{
    tbl_iproute_node_add_fail_t* pst_iproute_node_add_fail = obj;
    FILE *fp = pargs->argv[0];
    uint32* p_is_first = (uint32*)(pargs->argv[1]);
    uint32* p_is_ipv4 = (uint32*)(pargs->argv[2]);
    char buf[CMD_BUF_64] = {0};
    char buf1[CMD_BUF_64] = {0};
    //int32 index = 0;

    if (*p_is_ipv4 && AF_INET == pst_iproute_node_add_fail->key.p.family)
    {
        if (*p_is_first)
        {
            sal_fprintf(fp, "destination\n");
            sal_fprintf(fp, "    nexthop\n");
            sal_fprintf(fp, "----------------------------------------------------------------------\n");
            *p_is_first = FALSE;
        }
    
        //for (index = 0; index < pst_iproute_node_add_fail->nh_group.nh_num; index++)
        {
            sal_fprintf(fp, "%s\n", prefix_val2str(buf, sizeof(buf), &pst_iproute_node_add_fail->key.p, TRUE));
            sal_fprintf(fp, "    %s\n", 
                //cdb_addr_ipv4_val2str(buf1, sizeof(buf1), &pst_iproute_node_add_fail->nh_group.nh[index].u.ipv4));
                cdb_addr_ipv4_val2str(buf1, sizeof(buf1), &pst_iproute_node_add_fail->key.nh_addr.ipv4));
        }
    }
    else if (!*p_is_ipv4 && AF_INET6 == pst_iproute_node_add_fail->key.p.family)
    {
        if (*p_is_first)
        {
            sal_fprintf(fp, "destination\n");
            sal_fprintf(fp, "    nexthop\n");
            sal_fprintf(fp, "----------------------------------------------------------------------\n");
            *p_is_first = FALSE;
        }
        
        //for (index = 0; index < pst_iproute_node_add_fail->nh_group.nh_num; index++)
        {
            sal_fprintf(fp, "%s\n", prefix_val2str(buf, sizeof(buf), &pst_iproute_node_add_fail->key.p, TRUE));
            sal_fprintf(fp, "    %s\n",
                //cdb_addr_ipv6_val2str(buf1, sizeof(buf1), &pst_iproute_node_add_fail->nh_group.nh[index].u.ipv6));
                cdb_addr_ipv6_val2str(buf1, sizeof(buf1), &pst_iproute_node_add_fail->key.nh_addr.ipv6));
        }
    }
    else
    {
        
    }

    return PM_E_NONE;
}

void
route_show_ip_route_add_fib_fail(cfg_cmd_para_t *para, uint32 is_ipv4)
{
    FILE                *fp = NULL;
    uint32 is_first = TRUE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return;
    }

    tbl_iter_args_t args;
    args.argv[0] = fp;
    args.argv[1] = &is_first;
    args.argv[2] = &is_ipv4;
    tbl_iproute_node_add_fail_iterate(_route_api_show_ip_route_add_fib_fail_interator, &args);
    if (!is_first)
    {
        //sal_fprintf(fp, "\n");
    }

    sal_fclose(fp);
    fp = NULL;
    
    return;
}

int32
route_show_ip_route_add_fib_fail_count(cfg_cmd_para_t *para)
{
    int32 add_fib_fail_count = 0;
    tbl_iproute_node_add_fail_count_master_t *p_master=NULL;
    FILE *fp = NULL;
    int rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_master = tbl_iproute_node_add_fail_count_get_master();
    if (NULL == fp)
    {
        rc = PM_E_CFG_PM_ERROR;
    }
    else if(NULL == p_master)
    {
        sal_fclose(fp);
        fp = NULL;
        rc = PM_E_CFG_PM_ERROR;
    }
    else
    {
        add_fib_fail_count = p_master->add_fail_list->count;
        sal_fprintf(fp, "Ip routes add-fib-fail count : %d",add_fib_fail_count);
        sal_fclose(fp);
        fp = NULL;
        rc = PM_E_NONE;
    }
    return rc;
}

int32
route_cmd_process_show_ipv6_route(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;

    if (FALSE == rt_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    switch (para->oper)
    {
    case CDB_OPER_SET:
        break;
    case CDB_OPER_GET:
        if (argc >=1 && 0 == sal_memcmp(argv[0], "add-fib-fail", sizeof("add-fib-fail")))
        {
            route_show_ip_route_add_fib_fail(para, FALSE);
        }
        else
        {
            return rc;
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
route_cmd_process_show_ip_route(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        break;
    case CDB_OPER_GET:
        if (argc >=1 && 0 == sal_memcmp(argv[0], "add-fib-fail", sizeof("add-fib-fail")))
        {
            route_show_ip_route_add_fib_fail(para, TRUE);
        }
        else if(argc >=1 && 0 == sal_memcmp(argv[0], "add-fib-fail-count", sizeof("add-fib-fail-count")))
        {
            rc = route_show_ip_route_add_fib_fail_count(para);
        }
        else 
        {
            return rc;
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
route_cmd_process_clear_ipv6_route_static(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;

    if (FALSE == rt_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        route_clear_ipv6_route_static_all();
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*                    0         1                   2                   3                      4        */
/*ping_ipv6/${dst}/out_if/${iface_name_dst}/src_address/${src_address}*/
int32
route_cmd_process_ping_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    int32 is_out_if_configure = FALSE;
    int32 is_src_address_configure = FALSE;
    addr_ipv6_t addr_ipv6_dest;
    addr_ipv6_t addr_ipv6_source;
    tbl_interface_t *p_db_if = NULL;
    _cmd_print_para(argv, argc);

    if (FALSE == rt_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (argc == 5)
        {
            sal_memset(&addr_ipv6_source, 0, sizeof(addr_ipv6_source));
            is_src_address_configure = TRUE;
            CFG_PARSER_IPV6_ADDR(argv[4], &addr_ipv6_source, para->p_rs);
            if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_source))
            {
                CFG_INVALID_PARAM_RET("The source IPv6 address must be a global unicast address");
            }
        }
        
        if (argc >= 3 && 0 != sal_strlen(argv[2]))
        {
            is_out_if_configure = TRUE;
        }
        
        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest) && is_out_if_configure == FALSE)
        {
            if (is_src_address_configure)
            {
                CFG_INVALID_PARAM_RET("The destination IPv6 address must be a global unicast address");
            }
            else
            {
                CFG_INVALID_PARAM_RET("Output interface required for linklocal address");
            }
        }

        /*fix bug 49378*/
        if (IN6_IS_ADDR_V4MAPPED(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET ("Not support ping IPv4-mapped IPv6 address");
        }

        if (IN6_IS_ADDR_MULTICAST(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support ping IPv6 multicast address");
        }

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest) && is_src_address_configure == TRUE)
        {
            CFG_INVALID_PARAM_RET("Source address not required for linklocal address");
        }

        if (!IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest)  && is_out_if_configure == TRUE)
        {
            CFG_INVALID_PARAM_RET("Output interface not required for global unicast address");
        }

        if (is_out_if_configure == TRUE)
        {
            p_db_if = tbl_interface_get_interface_by_name(argv[2]);
            if (p_db_if)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("LAG member port cannot be configured as output interface");
                }
                
                if (!(tbl_interface_is_running(p_db_if) && tbl_interface_is_up(p_db_if)))
                {
                    CFG_INVALID_PARAM_RET("connect: Network is unreachable");
                }
            }
            else
            {
                char ifname_ext[IFNAME_EXT_SIZE] = {0};
                CFG_INVALID_PARAM_RET("Interface %s does not exist", IFNAME_ETH2FULL(argv[2], ifname_ext));
            }
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*                    0   */
/*ping_ipv6/${dst}*/
int32
route_cmd_process_ping_mgmt_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    _cmd_print_para(argv, argc);
    int32 rc = 0;
    addr_ipv6_t addr_ipv6_dest;
    /*modified by chenc for bug 53945, 2019-10-17*/
#if 0
    if (FALSE == rt_api_is_ipv6_profile())
    {
        if (FALSE == rt_api_is_ipv6_profile_for_gg_gb())
        {
            CFG_CONFLICT_RET("No ipv6 resource, please change profile");
        }
    }
#endif
    /*chenc end*/
    sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
    CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

    if (IN6_IS_ADDR_MULTICAST(&addr_ipv6_dest))
    {
        CFG_INVALID_PARAM_RET("Not support ping IPv6 multicast address");
    }

    /*fix bug 49378*/
    if (IN6_IS_ADDR_V4MAPPED(&addr_ipv6_dest))
    {
        CFG_INVALID_PARAM_RET ("Not support ping IPv4-mapped IPv6 address");
    }

    return 0;
}

/*                      0      1               2                3                 4                    5                    6        */
/*traceroute/${dst}/tos/${traceroute_tos}/out_if/${iface_name}/src_address/${src_address}*/
int32
route_cmd_process_traceroute_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 is_out_if_configure = FALSE;
    int32 is_src_address_configure = FALSE;
    prefix_t addr_dest;
    addr_t addr_source;
    tbl_interface_t *p_db_if = NULL;
    char addr_buf[CMD_BUF_32];
    char addr_buf1[CMD_BUF_32];
    char addr_buf2[CMD_BUF_32];
    int32 flag = TRUE;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_conn = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    int32 i;
    prefix_t p;
    prefix_t q;
    uint8 prefixlen = 0;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
   _cmd_print_para(argv, argc);
   
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(addr_buf,0,sizeof(addr_buf));
        sal_memset(&addr_dest, 0, sizeof(addr_dest));
        addr_dest.family = AF_INET;
        CFG_PARSER_IP_ADDR(argv[0], &addr_dest.u.prefix4, para->p_rs);
        if (argc == 7)
        {
            is_src_address_configure = TRUE;
            sal_memset(&addr_source, 0, sizeof(addr_source));
            addr_source.family = AF_INET;
            CFG_PARSER_IP_ADDR(argv[6], &addr_source.u.prefix4, para->p_rs);
        }
        if (argc >= 5 && 0 != sal_strlen(argv[4]))
        {
            is_out_if_configure = TRUE;
        }
        if ( TRUE == is_out_if_configure )
        {
            p_db_if = tbl_interface_get_interface_by_name(argv[4]);
            if (p_db_if !=NULL)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("LAG member port cannot be configured as output interface");
                }
                
                if (!(tbl_interface_is_running(p_db_if) && tbl_interface_is_up(p_db_if)))
                {
                    CFG_INVALID_PARAM_RET("connect: Network is unreachable");
                }
            }
            else
            {
                char ifname_ext[IFNAME_EXT_SIZE] = {0};
                CFG_INVALID_PARAM_RET("Interface %s does not exist", IFNAME_ETH2FULL(argv[4], ifname_ext));
            }
        }
        /*choose the appropriate IPv4 address as the source address , by guoxd , 2019-09-17*/
        /*1.if user appoints the source address , use the user-specified address as the source ip address*/
         if ( TRUE == is_src_address_configure )
        {
            sal_memset(addr_buf,0,sizeof(addr_buf));
            if (FALSE == route_if_addr_exists(&addr_source))
            {
                CFG_CONFLICT_RET("IP address %s not configured", cdb_addr_val2str(addr_buf, CMD_BUF_32, &addr_source));
            }
            cdb_addr_val2str(addr_buf,CMD_BUF_32, &addr_source);
        }
        /* 2.if user appoints the interface,select the ip address of the same network segment with the destination address as the source address in the interface.
               if it does not exist , the main ip address of the interface is selected.*/
        else if(TRUE == is_out_if_configure)
        {
            p_db_rtif = tbl_route_if_get_route_if_by_name(p_db_if->key.name);
            if(p_db_rtif ==NULL) 
            {
                char ifname_ext[IFNAME_EXT_SIZE] = {0};
                CFG_INVALID_PARAM_RET("Interface %s does not configure", IFNAME_ETH2FULL(argv[4], ifname_ext));
            }
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode)
            {
                if (NULL != p_db_conn)
                {
                    flag = TRUE;
                    prefixlen = p_db_conn->key.address.prefixlen;
                    rt_api_mask_apply(&p,&p_db_conn->key.address, prefixlen);
                    rt_api_mask_apply(&q,&addr_dest, prefixlen);
                    sal_memset(addr_buf1,0,sizeof(addr_buf1));
                    sal_memset(addr_buf2,0,sizeof(addr_buf2));
                    cdb_prefix_val2str(addr_buf1,CMD_BUF_32, &p);
                    cdb_prefix_val2str(addr_buf2,CMD_BUF_32, &q);
                    for(i=0;addr_buf1[i]!='#' && addr_buf2[i]!='#' && addr_buf1[i]!='\0' && addr_buf2[i]!='\0';i++)
                    {
                        if(addr_buf1[i] != addr_buf2[i])
                        {
                            flag = FALSE;
                            break;
                        }
                    }
                    if (TRUE == flag) 
                    {
                        sal_memset(&addr_source, 0, sizeof(addr_t));
                        addr_source.family = p_db_conn->key.address.family;
                        sal_memcpy(&addr_source.u,&p_db_conn->key.address.u,sizeof(addr_u));
                        sal_memset(addr_buf, 0, sizeof(addr_buf));
                        cdb_addr_val2str(addr_buf,CMD_BUF_32, &addr_source);
                        break;
                    }
                    if (!GLB_FLAG_ISSET(p_db_conn->flags, RT_IF_ADDR_SECONDARY))
                    {
                        sal_memset(&addr_source, 0, sizeof(addr_t));
                        addr_source.family = p_db_conn->key.address.family;
                        sal_memcpy(&addr_source.u,&p_db_conn->key.address.u,sizeof(addr_u));
                        sal_memset(addr_buf, 0, sizeof(addr_buf));
                        cdb_addr_val2str(addr_buf,CMD_BUF_32, &addr_source);
                    }
                }
            }
            if(*addr_buf == '\0'){
                char ifname_ext[IFNAME_EXT_SIZE] = {0};
                CFG_INVALID_PARAM_RET("Interface %s does not configure properly ip address", IFNAME_ETH2FULL(argv[4], ifname_ext));
            }
        }
        if(TRUE == is_out_if_configure || TRUE ==is_src_address_configure) 
        {
            fp = sal_fopen("/tmp/clearXXXXXX", "w+");
            sal_fprintf(fp, "%s",addr_buf);
            sal_fclose(fp);
            fp = NULL;
        }
        break;
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return rc;
}

/*                             0         1                   2                   3                      4         */
/*traceroute_ipv6/${dst}/out_if/${iface_name_dst}/src_address/${src_address}*/
int32
route_cmd_process_traceroute_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    int32 is_out_if_configure = FALSE;
    int32 is_src_address_configure = FALSE;
    addr_ipv6_t addr_ipv6_dest;
    addr_ipv6_t addr_ipv6_source;
    tbl_interface_t *p_db_if = NULL;
    _cmd_print_para(argv, argc);

    if (FALSE == rt_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support traceroute IPv6 linklocal address");
        }
        
        if (argc == 5)
        {
            sal_memset(&addr_ipv6_source, 0, sizeof(addr_ipv6_source));
            is_src_address_configure = TRUE;
            CFG_PARSER_IPV6_ADDR(argv[4], &addr_ipv6_source, para->p_rs);
            if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_source))
            {
                CFG_INVALID_PARAM_RET("The source IPv6 address must be a global unicast address");
            }
        }

        if (argc >= 3 && 0 != sal_strlen(argv[2]))
        {
            is_out_if_configure = TRUE;
        }

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest) && is_out_if_configure == FALSE)
        {
            if (is_src_address_configure)
            {
                CFG_INVALID_PARAM_RET("The destination IPv6 address must be a global unicast address");
            }
            else
            {
                CFG_INVALID_PARAM_RET("Output interface required for linklocal address");
            }
        }

        if (IN6_IS_ADDR_V4MAPPED(&addr_ipv6_dest))
        {
            //CFG_INVALID_PARAM_RET ("Not support ping IPv4-mapped IPv6 address");
        }

        if (IN6_IS_ADDR_MULTICAST(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support traceroute IPv6 multicast address");
        }

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest) && is_src_address_configure == TRUE)
        {
            CFG_INVALID_PARAM_RET("Source address not required for linklocal address");
        }

        if (!IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest)  && is_out_if_configure == TRUE)
        {
            //CFG_INVALID_PARAM_RET("Output interface not required for global unicast address");
        }

        if (is_out_if_configure == TRUE)
        {
            p_db_if = tbl_interface_get_interface_by_name(argv[2]);
            if (p_db_if)
            {
                if (INTERFACE_IS_LAG_MEMBER(p_db_if))
                {
                    CFG_INVALID_PARAM_RET("LAG member port cannot be configured as output interface");
                }
                
                if (!(tbl_interface_is_running(p_db_if) && tbl_interface_is_up(p_db_if)))
                {
                    CFG_INVALID_PARAM_RET("connect: Network is unreachable");
                }
            }
            else
            {
                char ifname_ext[IFNAME_EXT_SIZE] = {0};
                CFG_INVALID_PARAM_RET("Interface %s does not exist", IFNAME_ETH2FULL(argv[2], ifname_ext));
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
route_str_is_ipv6_addr(char *str)
{
    int32 rc = 0;
    addr_ipv6_t addr_ipv6_dest;
    sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
    
     rc = sal_inet_pton(AF_INET6, str, &addr_ipv6_dest); 
    if (rc != 1) 
    { 
        return FALSE;
    } 
    
    return TRUE;
}

/*                                       0   */
/*traceroute_mgmt_ipv6/${dst}*/
int32
route_cmd_process_traceroute_mgmt_ipv6_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    //int32 is_out_if_configure = FALSE;
    //int32 is_src_address_configure = FALSE;
    addr_ipv6_t addr_ipv6_dest;
    //tbl_interface_t *p_db_if = NULL;
    int32 dst_is_ipv6_address = FALSE;
    _cmd_print_para(argv, argc);

    if (route_str_is_ipv6_addr(argv[0]))
    {
        dst_is_ipv6_address = TRUE;
    }
    
    if (TRUE == dst_is_ipv6_address && FALSE == rt_api_is_ipv6_profile())
    {
        CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
    }

    if (!dst_is_ipv6_address)
    {
        return PM_E_NONE;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (IN6_IS_ADDR_MULTICAST(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support traceroute IPv6 multicast address");
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*cdbctl update/cdb/l3/tftp_check/${serverip}*/
int32
route_cmd_process_tftp_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    addr_ipv6_t addr_ipv6_dest;
    _cmd_print_para(argv, argc);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support tftp IPv6 linklocal address");
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*cdbctl update/cdb/l3/ftp_check/${host}*/
int32
route_cmd_process_ftp_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    addr_ipv6_t addr_ipv6_dest;
    _cmd_print_para(argv, argc);

    if (!route_str_is_ipv6_addr(argv[0]))
    {
        return PM_E_NONE;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support ftp IPv6 linklocal address");
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*cdbctl update/cdb/l3/ssh_check/${ipaddr6}*/
int32
route_cmd_process_ssh_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    addr_ipv6_t addr_ipv6_dest;
    _cmd_print_para(argv, argc);

    if (argc == 0)
    {
        return PM_E_NONE;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support ssh IPv6 linklocal address");
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/*cdbctl update/cdb/l3/telnet_check/${dst}*/
int32
route_cmd_process_telnet_check(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    addr_ipv6_t addr_ipv6_dest;
    _cmd_print_para(argv, argc);

    if (!route_str_is_ipv6_addr(argv[0]))
    {
        return PM_E_NONE;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        sal_memset(&addr_ipv6_dest, 0, sizeof(addr_ipv6_dest));
        CFG_PARSER_IPV6_ADDR(argv[0], &addr_ipv6_dest, para->p_rs);

        if (IN6_IS_ADDR_LINKLOCAL(&addr_ipv6_dest))
        {
            CFG_INVALID_PARAM_RET("Not support telnet IPv6 linklocal address");
        }
        
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
_route_cmd_show_by_list(void *obj, tbl_iter_args_t *pargs)
{
    char buf[CMD_BUF_256];
    tbl_interface_t* p_tbl_if_t = NULL;
    tbl_kernel_if_t *p_k_if_t = obj;
    int ret = PM_E_NONE;
    FILE * fp = NULL;

    if (0x0 == p_k_if_t->ip_addr.s_addr)
    {
        if ( 0 >= p_k_if_t->masklen ||  32 < p_k_if_t->masklen)
        {
            return PM_E_INVALID_PARAM;
        }
    }
    else
    {
        p_tbl_if_t = tbl_interface_get_interface_by_name(p_k_if_t->name);
        if (NULL == p_tbl_if_t)
        {
            return PM_E_NOT_EXIST;
        }
        if(tbl_interface_is_running(p_tbl_if_t))
        {
            sal_fprintf(fp, "%-10s %-16s %-10s \n", p_k_if_t->name,
                        cdb_addr_ipv4_val2str(buf, CMD_BUF_256, &p_k_if_t->ip_addr), "UP");
        }
        tbl_interface_is_running(p_tbl_if_t);
        {
            sal_fprintf(fp, "%-10s\n", "UP");
        }
        tbl_interface_is_up(p_tbl_if_t);
    }

    return ret;
}

static int32
_route_cmd_get_kernel_if(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_kernel_if_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_route_cmd_get_route_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_route_global_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
route_cmd_process_kernel_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t      *p_node             = NULL;
    tbl_kernel_if_t *pst_kernel_if      = NULL;
    field_parse_t   field;
    uint32          is_running = 0;
    char            if_name[IFNAME_SIZE];
    int32           rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_KERNEL_IF);
    sal_memset(if_name, 0, sizeof(if_name));
    sal_memset(&field, 0, sizeof(field_parse_t));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _route_cmd_get_kernel_if(para, p_node, &field);
        break;

    case CDB_OPER_SET:
         if (3 == argc)
        {

            if (0 == sal_strcmp(argv[0], "running_set"))
            {
                is_running = sal_atoi(argv[1]);
                sal_strcpy(if_name, argv[2]);

                pst_kernel_if = tbl_kernel_if_get_by_name(if_name);
                if (NULL == pst_kernel_if)
                {
                    return rc;
                }

                rc = lib_netlink_link_set(pst_kernel_if->key.ifindex, is_running);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter!");
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
_rt_api_set_ipv6_enable(uint32 ipv6_enable, tbl_route_global_t  *p_db_rt_glb)
{
    int32 rc = PM_E_NONE;
    //FILE *fp = NULL;

    if (ipv6_enable)
    {
        system("sysctl -w net.ipv6.conf.all.forwarding=1");
    }
    else
    {
        system("sysctl -w net.ipv6.conf.all.forwarding=0");
    }

    #if 0
    fp = fopen (proc_ipv6_forwarding, "w");

    if (fp == NULL)
    {
        return PM_E_FAIL;
    }

    if (ipv6_enable == 0)
    {
        fprintf (fp, "0\n");
    }
    else 
    {
        fprintf (fp, "1\n");
    }

    fclose (fp);
    #else
    // TODO: call frr ipv6 forwarding
    
    #endif

    p_db_rt_glb->ipv6_enable = ipv6_enable;
    PM_E_RETURN(tbl_route_global_set_route_global_field(p_db_rt_glb, TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE));
    
    return PM_E_NONE;
}

int32
_rt_api_set_ip_redirects_global_en(uint32 ip_redirects_global_en, tbl_route_global_t  *p_db_rt_glb)
{
    int32 rc = PM_E_NONE;

    p_db_rt_glb->ip_redirects_global_en = ip_redirects_global_en;
    PM_E_RETURN(tbl_route_global_set_route_global_field(p_db_rt_glb, TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN));
    
    return PM_E_NONE;
}

void
route_set_ipv6_enable(uint32 ipv6_enable)
{
    tbl_route_global_t *p_db_rt_glb = tbl_route_global_get_route_global();
    if (p_db_rt_glb)
    {
        _rt_api_set_ipv6_enable(ipv6_enable, p_db_rt_glb);
    }
}

static int32
_rt_cmd_set_ipv6_enable(uint32 ipv6_enable, tbl_route_global_t  *p_db_rt_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (ipv6_enable == p_db_rt_glb->ipv6_enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(_rt_api_set_ipv6_enable(ipv6_enable, p_db_rt_glb));

    return rc;
}

static int32
_rt_cmd_set_ip_redirects_global_en(uint32 ip_redirects_global_en, tbl_route_global_t  *p_db_rt_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (ip_redirects_global_en == p_db_rt_glb->ip_redirects_global_en)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(_rt_api_set_ip_redirects_global_en(ip_redirects_global_en, p_db_rt_glb));

    return rc;
}

int32
route_cmd_process_route_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_route_global_t   *p_db_rt_glb      = tbl_route_global_get_route_global();
    cdb_node_t            *p_node           = NULL;
    field_parse_t           field;
    int32                      rc = PM_E_NONE;
    cdb_field_t              *p_field = NULL;
    uint32                     i = 0;
    int32                      field_id = 0;
    uint32               ipv6_enable= FALSE;
    uint32               ip_redirects_global_en= FALSE;

    CFG_DBG_FUNC();
    if (NULL == p_db_rt_glb)
    {
       return PM_E_FAIL;
    }

    p_node = cdb_get_tbl(TBL_ROUTE_GLOBAL);
    sal_memset(&field, 0, sizeof(field_parse_t));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _route_cmd_get_route_global(para, p_node, &field);
        break;

    case CDB_OPER_SET:
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
        case TBL_ROUTE_GLOBAL_FLD_GRATUITOUS_ARP_LEARN_EN:
            p_db_rt_glb->gratuitous_arp_learn_en = sal_atoi(argv[1]);
            rc = tbl_route_global_set_route_global_field(p_db_rt_glb, field_id);
            break;

        case TBL_ROUTE_GLOBAL_FLD_ND_HOP_LIMIT:
            if (FALSE == rt_api_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }
            
            p_db_rt_glb->nd_hop_limit = sal_atoi(argv[1]);
            rc = nd_hop_limit_set(p_db_rt_glb->nd_hop_limit, para);
            break;

        case TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE:
            if (FALSE == rt_api_is_ipv6_profile())
            {
                CFG_CONFLICT_RET("No ipv6 resource, please change profile");    
            }
            
            ipv6_enable = sal_atoi(argv[1]);
            rc = _rt_cmd_set_ipv6_enable(ipv6_enable, p_db_rt_glb, para);
            break;

        case TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN:
            ip_redirects_global_en = sal_atoi(argv[1]);
            rc = _rt_cmd_set_ip_redirects_global_en(ip_redirects_global_en, p_db_rt_glb, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Invalid parameter %s", argv[i]);
            break;
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_rt_cmd_iproute_node_add_fail_cmd_get_iproute_node_add_fail
    (tbl_iproute_node_add_fail_t *pst_iproute_node_add_fail, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (NULL == pst_iproute_node_add_fail)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_iproute_node_add_fail_dump_one, &args);
    }

    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

static int32
_rt_cmd_iproute_node_add_fail_count_cmd_get_iproute_node_add_fail_count
    (tbl_iproute_node_add_fail_count_t *pst_iproute_node_add_fail_count, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (NULL == pst_iproute_node_add_fail_count)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_iproute_node_add_fail_count_dump_one, &args);
    }

    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
rt_cmd_iproute_node_add_fail_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    p_node = cdb_get_tbl(TBL_IPROUTE_NODE_ADD_FAIL);
    CFG_DBG_FUNC();

    tbl_iproute_node_add_fail_t *pst_iproute_node_add_fail = NULL;
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;
        
    case CDB_OPER_SET:
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _rt_cmd_iproute_node_add_fail_cmd_get_iproute_node_add_fail(pst_iproute_node_add_fail, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
rt_cmd_iproute_node_add_fail_count_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    p_node = cdb_get_tbl(TBL_IPROUTE_NODE_ADD_FAIL_COUNT);
    CFG_DBG_FUNC();

    tbl_iproute_node_add_fail_count_t *pst_iproute_node_add_fail_count = NULL;
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;
        
    case CDB_OPER_SET:
        break;
        
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _rt_cmd_iproute_node_add_fail_count_cmd_get_iproute_node_add_fail_count(pst_iproute_node_add_fail_count, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

