#include "proto.h"
#include "hsrv_inc.h"
#include "hsrv_acl.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_acl_l4_port_range_define.h"
#include "gen/tbl_acl_l4_port_range.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_tap_group_define.h"
#include "gen/tbl_tap_group.h"
/* modified by liwh for bug 44506, 2017-06-28 */
#include "gen/tbl_ospf_network_define.h"
#include "gen/tbl_ospf_network.h"
/*liwh end */
/* support vrrp modified by liwh for bug 45215, 2017-09-17 */
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
/* liwh end */
#include "gen/tbl_ospf_area_range_define.h"
#include "gen/tbl_ospf_area_range.h"
#include "gen/tbl_ospf_area_auth_define.h"
#include "gen/tbl_ospf_area_auth.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_track_global_define.h"
#include "gen/tbl_track_global.h"
#include "gen/tbl_mrt_define.h"
#include "gen/tbl_mrt.h"
#include "gen/tbl_nd_fib_define.h"
#include "gen/tbl_nd_fib.h"
#include "igmp_define.h"
#include "gen/tbl_iproute_node_add_fail_count_define.h"
#include "gen/tbl_iproute_node_add_fail_count.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"
#include "glb_hash_field_value_define.h"
#include "glb_openflow_define.h"

#ifdef TSINGMA    
/*begin add by taocy for BUG52801*/
#include "gen/tbl_nat_global_define.h"
#include "gen/tbl_nat_global.h"
#include "gen/tbl_nat_rule_define.h"
#include "gen/tbl_nat_rule.h"
#include "gen/tbl_nat_pool_define.h"
#include "gen/tbl_nat_pool.h"
#include "gen/tbl_nat_session_limit_define.h"
#include "gen/tbl_nat_session_limit.h"
#include "gen/tbl_nat_rtif_inside_define.h"
#include "gen/tbl_nat_rtif_inside.h"
#include "gen/tbl_nat_rtif_outside_define.h"
#include "gen/tbl_nat_rtif_outside.h"
/*end add by taocy for BUG52801*/
#endif

/* modified by liwh for bug 53010, 2019-07-22 */
#if defined (GOLDENGATE) || defined (GREATBELT)
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"
#endif
/* liwh end */

#define HSRV_STM_STATIC_MAX_CNT           1024

#define SHOW_SYS_SPEC_ONE(string, num1, num2) \
do{\
    if (current)\
        sal_fprintf(fp,"%-42s: %-d/%-d\n", (string), (num1), (num2));\
    else\
        sal_fprintf(fp,"%-42s: %-d\n", (string), (num2));\
}while(0)

#define SHOW_SYS_SPEC(string, num1, num2) \
do{\
    if (current)\
        sal_fprintf(fp,"    %-38s: %-d/%-d\n", (string), (num1), (num2));\
    else\
        sal_fprintf(fp,"    %-38s: %-d\n", (string), (num2));\
}while(0)

#define SHOW_SYS_SPEC_3INT(string, num1, num2, num3) \
do{\
    if (current)\
        sal_fprintf(fp,"    %-38s: (%d+%d)/%d\n", (string), (num1), (num2),(num3));\
    else\
        sal_fprintf(fp,"    %-38s: %-d\n", (string), (num3));\
}while(0)

#define SHOW_SYS_SPEC_3INT_V6(string, num1, num2, num3) \
do{\
    if (current)\
        sal_fprintf(fp,"    %-38s: (%d+%d*2)/%d\n", (string), (num1), (num2),(num3));\
    else\
        sal_fprintf(fp,"    %-38s: %-d\n", (string), (num3));\
}while(0)

#define SHOW_SYS_SPEC_4INT(string, num1, num2, num3, num4) \
do{\
    if (current)\
        sal_fprintf(fp,"    %-38s: (%d+%d+%d)/%d\n", (string), (num1), (num2),(num3), (num4));\
    else\
        sal_fprintf(fp,"    %-38s: %-d\n", (string), (num4));\
}while(0)

#define SHOW_SYS_SPEC_SUB(string, num1, num2) \
do{\
    if (current)\
        sal_fprintf(fp,"        %-34s: %-d/%-d\n", (string), (num1), (num2));\
    else\
        sal_fprintf(fp,"        %-34s: %-d\n", (string), (num2));\
}while(0)


/*added by yejl for ip/mac/ipv6 acl*/
int32
hsrv_acl_get_mac_ip_ipv6_acl_number(uint32 *ipacl_num, uint32 *macacl_num, uint32 *ipv6acl_num)
{
    tbl_acl_config_t * p_db_acl_config = NULL;
    tbl_acl_config_master_t     * p_acl_cfg_master  = tbl_acl_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    uint32 ip_num = 0;
    uint32 mac_num = 0;
    uint32 ipv6_num = 0;

    CTCLIB_SLIST_LOOP(p_acl_cfg_master->acl_config_list, p_db_acl_config, listnode)
    {
        if (GLB_ACL_CONFIG_TYPE_NORMAL == p_db_acl_config->type_identifying)
        {
            ip_num++;
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == p_db_acl_config->type_identifying)
        {
            mac_num++;
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == p_db_acl_config->type_identifying)
        {   
            ipv6_num++;       
        }
    }
    *ipacl_num = ip_num;
    *macacl_num = mac_num;
    *ipv6acl_num = ipv6_num;
    return HSRV_E_NONE;
}

int32
hsrv_acl_get_mac_ip_ipv6_ace_number(uint32 *ipace_num, uint32 *macace_num, uint32 *ipv6ace_num)
{
    tbl_ace_config_t * p_db_ace_config = NULL;
    tbl_ace_config_master_t     * p_ace_cfg_master  = tbl_ace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    uint32 ip_num = 0;
    uint32 mac_num = 0;
    uint32 ipv6_num = 0;

    CTCLIB_SLIST_LOOP(p_ace_cfg_master->ace_config_list, p_db_ace_config, listnode)
    {
        if (GLB_ACL_CONFIG_TYPE_NORMAL == p_db_ace_config->acl_type)
        {
            ip_num++;
        }
        else if (GLB_ACL_CONFIG_TYPE_MAC == p_db_ace_config->acl_type)
        {
            mac_num++;
        }
        else if (GLB_ACL_CONFIG_TYPE_IPV6 == p_db_ace_config->acl_type)
        {   
            ipv6_num++;        
        }
    }
    *ipace_num = ip_num;
    *macace_num = mac_num;
    *ipv6ace_num = ipv6_num;
    return HSRV_E_NONE;
}


int32
hsrv_sys_acl_entry_get_count(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    uint32 *num_ingress = pargs->argv[0];
    uint32 *num_egress = pargs->argv[1];
    uint32 *num_ingress6 = pargs->argv[2];
    uint32 *num_egress6 = pargs->argv[3];
    uint32 *num_global_ingress = pargs->argv[4];
    uint32 *num_global_ingress6 = pargs->argv[5];

    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            *num_ingress6 += 1;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == hsrv_acl_stm_is_ipv6_profile())
            {
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    *num_ingress += 1;
                    *num_ingress6 += 1;
                }
                else
                {
                    *num_ingress += 1;
                }
            }
            else
            {
                *num_ingress += 1;
            }
        }
        else
        {
            *num_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            *num_egress6 += 1;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == hsrv_acl_stm_is_ipv6_profile())
            {
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    *num_egress += 1;
                    *num_egress6 += 1;
                }
                else
                {
                    *num_egress += 1;
                }
            }
            else
            {
                *num_egress += 1;
            }
        }
        else
        {
            *num_egress += 1;
        }
    }
    if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            *num_global_ingress6 += 1;
        }
        else
        {
            *num_global_ingress += 1;
        }
    }
    return HSRV_E_NONE;
}

int32
hsrv_sys_acl_entry_get_count_1(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    acl_entry_count_t *entry_count = pargs->argv[0];
    
    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_ingress6 += 1;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == hsrv_acl_stm_is_ipv6_profile())
            {   
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    entry_count->cur_ingress6 += 1;
                    entry_count->cur_ingress += 1;
                }
                else
                {
                    entry_count->cur_ingress += 1;
                }
            }
            else
            {
                entry_count->cur_ingress += 1;
            }
        }
        else
        {
            entry_count->cur_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_egress6 += 1;
        }
        else if (p_acl_entry->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == hsrv_acl_stm_is_ipv6_profile())
            {   
                if (!GLB_FLAG_ISSET(p_acl_entry->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_acl_entry->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    entry_count->cur_egress6 += 1;
                    entry_count->cur_egress += 1;
                }
                else
                {
                    entry_count->cur_egress += 1;
                }
            }
            else
            {
                entry_count->cur_egress += 1;
            }
        }
        else
        {
            entry_count->cur_egress += 1;
        }     
    }

	if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_worm_filter6 += 1;
        }
        else
        {
            entry_count->cur_worm_filter += 1;
        }
    }

    return HSRV_E_NONE;
}



int32
hsrv_sys_acl_entry_get_entry_dir_count(uint32* cnt_ingress, uint32* cnt_egress, uint32* cnt_ingress6,
                         uint32* cnt_egress6, uint32* cnt_worm_filter, uint32* cnt_worm_filter6)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = cnt_ingress;
    args.argv[1] = cnt_egress;
    args.argv[2] = cnt_ingress6;
    args.argv[3] = cnt_egress6;
    args.argv[4] = cnt_worm_filter;
    args.argv[5] = cnt_worm_filter6;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)hsrv_sys_acl_entry_get_count, &args);
    return HSRV_E_NONE;
}

int32
hsrv_sys_acl_entry_get_entry_dir_count_1(acl_entry_count_t *entry_count)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = entry_count;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)hsrv_sys_acl_entry_get_count_1, &args);
    return HSRV_E_NONE;
}
/*ended by yejl*/


int32
hsrv_cmd_map_sys_spec_type(char **argv, int32 argc, cfg_cmd_para_t *para, int32 *type, uint32 *current)
{
    tbl_chassis_t* p_chassis = NULL;
    if (argc)
    {
        if (0 == sal_strcmp(argv[0], "default"))
        {
            *type = GLB_STM_MODE_DEFAULT;
        }
        else if (0 == sal_strcmp(argv[0], "ipv4"))
        {
            *type = GLB_STM_MODE_IPUCV4;
        }
        else if (0 == sal_strcmp(argv[0], "vlan"))
        {
            *type = GLB_STM_MODE_VLAN;
        }
        else if (0 == sal_strcmp(argv[0], "ipv6"))
        {
            *type = GLB_STM_MODE_IPUCV6;
        }
        else if (0 == sal_strcmp(argv[0], "nvgre"))
        {
            *type = GLB_STM_MODE_NVGRE;
        }
        else if (0 == sal_strcmp(argv[0], "l2vpn"))
        {
            *type = GLB_STM_MODE_L2VPN;
        }
        else if (0 == sal_strcmp(argv[0], "openflow-default"))
        {
            *type = GLB_STM_MODE_OF_DEFAULT;
        }
        else if (0 == sal_strcmp(argv[0], "openflow-hybrid"))
        {
            *type = GLB_STM_MODE_OF_HYBRID;
        }
        else if (0 == sal_strcmp(argv[0], "openflow-ipv6"))
        {
            *type = GLB_STM_MODE_OF_IPV6;
        }
        else if (0 == sal_strcmp(argv[0], "flow6"))
        {
            *type = GLB_STM_MODE_TAP_IPV4_IPV6;
        }
        else if (0 == sal_strcmp(argv[0], "current"))
        {
            p_chassis = tbl_chassis_get_chassis();
            *type = p_chassis->cur_stm_mode;
            *current = TRUE;
        }
        else if (0 == sal_strcmp(argv[0], "next"))
        {
            p_chassis = tbl_chassis_get_chassis();
            *type = p_chassis->next_stm_mode;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid stm profile type: %s", argv[0]);
        }
        return PM_E_NONE;
    }
    CFG_INVALID_PARAM_RET("Stm profile type is not specified");
}

/* flag 0:policer profile;flag 1:drop profile;flag 2:queue shape;flag 3:port shape; flag 4:policer action profile;flag 5:mpls label */
uint32
hsrv_get_qos_used_profile_num(uint32 flag)
{
    uint32 num = 0;
    uint32 index =0;
    tbl_qos_policer_profile_master_t *p_policer_master = tbl_qos_policer_profile_get_master();
    tbl_qos_drop_profile_master_t *p_drop_master = tbl_qos_drop_profile_get_master();
    tbl_qos_queue_shape_profile_master_t *p_queue_shape_master = tbl_qos_queue_shape_profile_get_master();
    tbl_qos_port_shape_profile_master_t *p_port_shape_master = tbl_qos_port_shape_profile_get_master();
    tbl_qos_policer_action_profile_master_t *p_policer_action_master = tbl_qos_policer_action_profile_get_master();
    tbl_qos_mpls_domain_master_t *p_mpls_domain_master = tbl_qos_mpls_domain_get_master();
    tbl_qos_queue_shape_profile_t *p_db_qos_queue_shape_profile = NULL;
    tbl_qos_port_shape_profile_t *p_db_qos_port_shape_profile = NULL;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    tbl_qos_drop_profile_t *p_db_qos_drop_profile = NULL;
    tbl_qos_policer_action_profile_t *p_db_qos_action_profile = NULL;

    switch(flag)
    {
    case 0:
        //policer
        for (index = 0; index < GLB_QOS_POLICER_PROFILE_NUM; index++)
        {
            p_db_qos_policer_profile = p_policer_master->qos_policer_profile_array[index];
            if (NULL == p_db_qos_policer_profile)
            {
                continue;
            }
            else
            {
                num ++;
            }
        }
        break;
    case 1:
        //drop
        for (index = 0; index < GLB_QOS_DROP_PROFILE_NUM; index++)
        {
            p_db_qos_drop_profile = p_drop_master->qos_drop_profile_array[index];
            if ((NULL == p_db_qos_drop_profile) || (0 == sal_strcmp(p_db_qos_drop_profile->name, GLB_QOS_DEF_WTD_PROFILE))
                                                              || (0 == sal_strcmp(p_db_qos_drop_profile->name, GLB_QOS_DEF_WRED_PROFILE)))
            {
                continue;
            }
            else
            {
                num ++;
            }
        }
        break;
    case 2:
        //queue shape
        for (index = 0; index < GLB_QOS_SHAPE_PROFILE_NUM; index++)
        {
            p_db_qos_queue_shape_profile = p_queue_shape_master->qos_queue_shape_profile_array[index];
            if ((NULL == p_db_qos_queue_shape_profile) || (0 == sal_strcmp(p_db_qos_queue_shape_profile->name, GLB_QOS_DEF_SCHEDULER_PROFILE)))
            {
                continue;
            }
            else
            {
                num ++;
            }
        }
        break;
    case 3:
        //port shape
        for (index = 0; index < GLB_QOS_PORT_SHAPE_PROFILE_NUM; index++)
        {
            p_db_qos_port_shape_profile = p_port_shape_master->qos_port_shape_profile_array[index];
           if ((NULL == p_db_qos_port_shape_profile) || (0 == sal_strcmp(p_db_qos_port_shape_profile->name, GLB_QOS_DEF_PORT_SHAPE_PROFILE)))
           {
                continue;
           }
           else
           {
               num++;
           }
        }
        break;
    case 4:
        //policer action profile
        for (index = 0; index < GLB_QOS_POLICER_ACTION_PROFILE_NUM; index++)
        {
            p_db_qos_action_profile = p_policer_action_master->qos_policer_action_profile_array[index];
            if (NULL == p_db_qos_action_profile)
            {
                continue;
            }
            else
            {
                num ++;
            }
        }
        break;
    case 5:
        num = p_mpls_domain_master->qos_mpls_domain_hash->count;
        break;
    default:
        break;        
    }
    
    return num;
}

uint32
hsrv_get_lag_num()
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_if_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 lag_num = 0;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_if_lag, listnode)
    {
        if (!INTERFACE_IS_LAG_GROUP(p_if_lag))
        {
            /* bypass Non-LAG */
            continue;
        }
        lag_num++;
    }

    return lag_num;
}

uint32
hsrv_get_l4_port_num(void)
{
    uint32 i   = 0;
    uint32 num = 0;
    tbl_acl_l4_port_range_key_t key;
    tbl_acl_l4_port_range_t    *ptr;

    for(i = 0;  i < GLB_ACL_L4PORT_ACE_NUM_MAX; i++)
    {
        key.id = i;
        ptr = tbl_acl_l4_port_range_get_acl_l4_port_range(&key);

        if(NULL != ptr && ptr->cnt >= 1)
        {
            num++;
        }
    }

    return num;
}

/*Modified by yejl for bug 48016, 2018-07-16*/
uint32
hsrv_get_l2protocol_entry_num(void)
{
    tbl_l2_protocol_master_t    * p_l2protocol_master  = tbl_l2_protocol_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_l2_protocol_t* p_db_l2_protocol = NULL;
    uint32 l2protocol_entry_num = 0;
    
    CTCLIB_SLIST_LOOP(p_l2protocol_master->l2_protocol_list, p_db_l2_protocol, listnode)
    {
        if (!GLB_IS_ZERO_MAC(p_db_l2_protocol->group_mac))
        {
            l2protocol_entry_num++;
        }
    }
    return l2protocol_entry_num;
}

int32 get_frr_ospf_route_num()
{
    int32 frr_ospf_route_num = 0;
    FILE *fp_console = NULL;
    char buf[256];

    fp_console = sal_fopen("/tmp/ospf_route_num", "r");
    if (fp_console)
    {
        while (sal_fgets(buf, 128, fp_console))
        {    
            sal_sscanf(buf, "%d", &frr_ospf_route_num);
        }
        sal_fclose(fp_console);
        return frr_ospf_route_num;
    }
    else
    {
        return -1;
    }
}

int32
hsrv_show_stm(tbl_sys_spec_t *p_sys_spec, FILE *fp, cfg_cmd_para_t *para, uint32 current, uint32 is_internal_debug)
{
    tbl_igsp_global_t           * p_igsp_glb        = tbl_igsp_global_get_igsp_global();
    tbl_mcfdb_master_t          * p_mcfdb_master    = tbl_mcfdb_get_master();
    tbl_vlan_master_t           * p_vlan_master     = tbl_vlan_get_master();
    tbl_arp_master_t            * p_arp_master      = tbl_arp_get_master();
    tbl_iproute_node_master_t   * p_ipfib_master    = tbl_iproute_node_get_master();
    tbl_iproute_node_add_fail_count_master_t   * p_iproute_node_add_fail_count_master    = tbl_iproute_node_add_fail_count_get_master();
    tbl_route_global_t          * p_rt_glb          = tbl_route_global_get_route_global();
    tbl_acl_entry_master_t      * p_acl_master      = tbl_acl_entry_get_master();
//    tbl_acl_config_master_t     * p_acl_cfg_master  = tbl_acl_config_get_master();
//    tbl_ace_config_master_t     * p_ace_cfg_master  = tbl_ace_config_get_master();
    tbl_brg_global_t            * p_brg_glb         = tbl_brg_global_get_brg_global();
    tbl_ipsg_fib_master_t       * p_db_ipsg         = tbl_ipsg_fib_get_master();
    tbl_dot1x_global_t          * p_db_glb          = tbl_dot1x_global_get_dot1x_global();
    tbl_macfilter_master_t      * p_macfilter_master = tbl_macfilter_get_master();
    tbl_nexthop_group_master_t  * p_nhg_master     = tbl_nexthop_group_get_master();
    tbl_static_rt_cnt_master_t  * p_static_rt_cnt   = tbl_static_rt_cnt_get_master();
    tbl_time_range_master_t     * p_time_range_master = tbl_time_range_get_master();
    tbl_mstp_instance_master_t  * p_mstp_inst_glb     = tbl_mstp_instance_get_master();
    tbl_sys_global_t            * p_sys_global      = tbl_sys_global_get_sys_global();
    tbl_arpacl_config_master_t  * p_arpacl_config_master   = tbl_arpacl_config_get_master();
    tbl_arpace_config_master_t  * p_arpace_config_master   = tbl_arpace_config_get_master();
    /* modified by liwh for bug 44506, 2017-06-28 */
    tbl_ospf_network_master_t   * p_ospf_master = tbl_ospf_network_get_master();
    /*liwh end */
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    tbl_vrrp_global_t           * p_db_vrrp_glb = tbl_vrrp_global_get_vrrp_global();
    /* liwh end */
    tbl_ospf_area_range_master_t   * p_ospf_area_range_master = tbl_ospf_area_range_get_master();
    tbl_ospf_area_auth_master_t *p_ospf_area_auth_master =  tbl_ospf_area_auth_get_master();
    tbl_ipsla_global_t          * p_db_ipsla_glb  = tbl_ipsla_global_get_ipsla_global();
    tbl_track_global_t          * p_db_track_glb  = tbl_track_global_get_track_global();
    tbl_evc_entry_master_t       *p_evc_master    = tbl_evc_entry_get_master();
    tbl_vlan_mapping_table_master_t     *p_vmt_master    = tbl_vlan_mapping_table_get_master();
    tbl_nd_fib_master_t * p_nd_fib_master = tbl_nd_fib_get_master();
    tbl_qos_global_t *p_qos_master = tbl_qos_global_get_qos_global();
#ifdef TSINGMA    
    tbl_hash_value_profile_master_t      * p_hash_value_pro_master = tbl_hash_value_profile_get_master();

    /*begin add by taocy for BUG52801*/
    tbl_nat_global_t                *p_nat_global               = tbl_nat_global_get_nat_global();
    tbl_nat_pool_master_t           *p_nat_pool_master          = tbl_nat_pool_get_master();
    tbl_nat_rule_master_t           *p_nat_rule_master          = tbl_nat_rule_get_master();
    tbl_nat_session_limit_master_t  *p_nat_session_limit_master = tbl_nat_session_limit_get_master();
    tbl_nat_rtif_inside_master_t    *p_nat_rtif_inside_master   = tbl_nat_rtif_inside_get_master();
    tbl_nat_rtif_outside_master_t   *p_nat_rtif_outside_master  = tbl_nat_rtif_outside_get_master();
    /*end add by taocy for BUG52801*/
#endif
/* modified by liwh for bug 53010, 2019-07-22 */
#if defined (GOLDENGATE) || defined (GREATBELT)
    tbl_cfm_ma_master_t      *p_ma_master = tbl_cfm_ma_get_master();
    tbl_cfm_lmep_master_t   *p_lmep_master = tbl_cfm_lmep_get_master();
    tbl_cfm_rmep_master_t   *p_rmep_master = tbl_cfm_rmep_get_master();
    tbl_cfm_md_master_t      *p_md_master = tbl_cfm_md_get_master();
    tbl_g8032_ring_master_t   *p_g8032_master = tbl_g8032_ring_get_master();
#endif
/* liwh end */

    tbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    uint32 num = 0;
    uint32 fdb_static = 0;
    uint32 fdb_dynamic = 0;
    uint32 security_dynamic = 0;
    uint32 security_static = 0;
    uint32 fdb_total = 0;
    #ifndef GREATBELT
    uint32 ace_entry_ingress = 0;
    uint32 ace_entry_egress = 0;
    uint32 ace_entry_ingress6 = 0;
    uint32 ace_entry_egress6 = 0;
    uint32 ace_entry_wormfilter = 0;
    uint32 ace_entry_wormfilter6 = 0;
    #endif
    uint32 policer_num = 0;
    uint32 drop_num = 0;
    uint32 queue_shape_num = 0;
    uint32 mpls_label_num = 0;
    uint32 policer_action_num = 0;
    uint32 port_shape_num = 0;
    uint32 l2protocol_entry_num = 0;
    uint32 ipacl_num = 0;
    uint32 macacl_num = 0;
    uint32 ipv6acl_num = 0;
    uint32 ipace_num = 0;
    uint32 macace_num = 0;
    uint32 ipv6ace_num = 0;
  #ifdef GREATBELT
    acl_entry_count_t entry_count = {0};
  #endif
    
    char tmp_str[38]={0};

    policer_num = hsrv_get_qos_used_profile_num(0);
    drop_num = hsrv_get_qos_used_profile_num(1);
    queue_shape_num = hsrv_get_qos_used_profile_num(2);
    port_shape_num = hsrv_get_qos_used_profile_num(3);
    policer_action_num = hsrv_get_qos_used_profile_num(4);
    mpls_label_num = hsrv_get_qos_used_profile_num(5);
    l2protocol_entry_num = hsrv_get_l2protocol_entry_num();

    if (!p_vlan_master || !p_arp_master || !p_acl_master || !p_sys_global
                || !p_db_ipsg || !p_db_glb || !p_nhg_master || !p_static_rt_cnt || !p_ospf_master || !p_db_vrrp_glb)
    {
        return PM_E_NOT_INITED;
    }

    if(GLB_STM_MODE_VLAN == p_sys_spec->key.type)
    {
        sal_fprintf(fp,"%s:\n", "vlan profile");
    }
    else if(GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
    {
        sal_fprintf(fp,"%s:\n", "IPv6 profile");
    }
    else
    {
        sal_fprintf(fp,"%s:\n", "default profile");
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_vlan_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        else
        {
            num ++;
        }
    }

    hsrv_fdb_get_count_number(&fdb_static, &security_static, &fdb_dynamic, &security_dynamic, &fdb_total);

    sal_fprintf(fp,"number of Ethernet features:\n");
    SHOW_SYS_SPEC("MSTP instances", p_mstp_inst_glb->inst_list->count, p_sys_spec->mstp_instance);
    SHOW_SYS_SPEC("VLAN forwarding instances", num, p_sys_spec->vlan_instance);
    SHOW_SYS_SPEC("Ucast MAC addresses", fdb_total, p_sys_spec->ucast_fdb);
    SHOW_SYS_SPEC_SUB("static MAC address", fdb_static + security_static, p_brg_glb->max_static_fdb);
    SHOW_SYS_SPEC_SUB("dynamic MAC address", fdb_dynamic + security_dynamic, p_sys_spec->ucast_fdb);
    SHOW_SYS_SPEC("MAC Filter entry", p_macfilter_master->macflt_list->count, p_sys_spec->mac_filter);
    SHOW_SYS_SPEC("Ethernet Virtual Connection", p_evc_master->evc_entry_list->count, GLB_VLAN_EVC_MAX_NUM);
    SHOW_SYS_SPEC("VLAN Mapping Table", p_vmt_master->vlan_mapping_table_list->count, GLB_VLAN_MAX_VLAN_MAP_TABLE);
    SHOW_SYS_SPEC("Applied VLAN Mapping Entry", p_brg_glb->vlan_mapping_entry_cnt, GLB_VLAN_MAPPING_ENTRY_MAX);
    SHOW_SYS_SPEC("L2protocol entry", l2protocol_entry_num, p_sys_spec->l2protocol_num);
    SHOW_SYS_SPEC("VLAN statistics", p_sys_global->vlan_stats_cnt, GLB_MAX_VLAN_STATS_ID);
    
    if (is_internal_debug)/*if interface is down, not consider*/
    {
        sal_fprintf(fp,"number of IP unicast features internal:\n");
        num = p_arp_master->arp_hash->count;
        num = p_rt_glb->current_host_routes;
        SHOW_SYS_SPEC("IPv4 host routes", num, p_sys_spec->host_routes);

        num = p_ipfib_master->rn_hash->count;
        num = p_rt_glb->current_remote_routes;
        SHOW_SYS_SPEC("Indirect IPv4 routes", num, p_sys_spec->indirect_routes);

        /* Static route belong to indirect routes but limit it to 1K */
        //num = p_static_rt_cnt->rt_cnt_hash->count;
        num = p_rt_glb->current_static_routes;
        /* For support all routes can be added by static route config */
        SHOW_SYS_SPEC("Static IPv4 routes", num, p_sys_spec->static_routes);

        num = p_rt_glb->current_ecmp_routes;
        //num = p_nhg_master->nhg_hash->count;
        sal_sprintf(tmp_str, "IPv4 ecmp groups(member maximum %d)", p_sys_spec->ecmp_member_maximum);
        SHOW_SYS_SPEC(tmp_str, num, p_sys_spec->ecmp_routes);

        num = p_db_ipsg->ipsg_fib_hash->count;
        SHOW_SYS_SPEC("IPv4 source guard entries", num, p_sys_spec->ipv4_source_guard);
        
        if(GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
        {
            sal_fprintf(fp,"number of IPv6 unicast features:\n");
            
            num = p_nd_fib_master->neighbor_list->count;
            SHOW_SYS_SPEC("IPv6 host routes", num, p_sys_spec->host_v6_routes);

            num = p_rt_glb->current_remote_v6_routes;
            SHOW_SYS_SPEC("Indirect IPv6 routes", num, p_sys_spec->indirect_v6_routes);

            //num = p_static_rt_cnt->rt_cnt_hash->count;
            num = p_rt_glb->current_static_v6_routes;
            SHOW_SYS_SPEC("Static IPv6 routes", num, p_sys_spec->static_v6_routes);

            num = p_rt_glb->current_ecmp_v6_routes;
            //num = p_nhg_master->nhg_hash->count;
            sal_sprintf(tmp_str, "IPv6 ecmp groups(member maximum %d)", p_sys_spec->ecmp_v6_member_maximum);
            SHOW_SYS_SPEC(tmp_str, num, p_sys_spec->ecmp_v6_routes);
        }
    }
    else
    {
        sal_fprintf(fp,"number of IP unicast features:\n");
        num = p_arp_master->arp_hash->count;
        num = p_rt_glb->current_host_routes;
        SHOW_SYS_SPEC("IPv4 host routes", num, p_sys_spec->host_routes);

        num = p_ipfib_master->rn_hash->count;
        //num = p_rt_glb->current_remote_routes;
        num = p_rt_glb->current_indirect_ipv4_route_num;

        int32 frr_ospf_route_num = 0;
        frr_ospf_route_num = get_frr_ospf_route_num();
        if (frr_ospf_route_num != -1)
        {
            num += frr_ospf_route_num;
        }

        int32 route_add_fail_count = p_iproute_node_add_fail_count_master->add_fail_hash->count;
        if (num >= route_add_fail_count)
        {
            num -= route_add_fail_count;
        }
        
        SHOW_SYS_SPEC("Indirect IPv4 routes", num, p_sys_spec->indirect_routes);

        /* Static route belong to indirect routes but limit it to 1K */
        //num = p_static_rt_cnt->rt_cnt_hash->count;
        num = p_rt_glb->current_static_routes;
        /* For support all routes can be added by static route config */
        SHOW_SYS_SPEC("Static IPv4 routes", num, p_sys_spec->static_routes);

        //num = p_rt_glb->current_ecmp_routes;
        //num = p_nhg_master->nhg_hash->count;
        num = p_rt_glb->current_ecmp_groups;
        sal_sprintf(tmp_str, "IPv4 ecmp groups(member maximum %d)", p_sys_spec->ecmp_member_maximum);
        SHOW_SYS_SPEC(tmp_str, num, p_sys_spec->ecmp_routes);

        num = p_db_ipsg->ipsg_fib_hash->count;
        SHOW_SYS_SPEC("IPv4 source guard entries", num, p_sys_spec->ipv4_source_guard);
        
        if(GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
        {
            sal_fprintf(fp,"number of IPv6 unicast features:\n");
            
            num = p_nd_fib_master->neighbor_list->count;
            SHOW_SYS_SPEC("IPv6 host routes", num, p_sys_spec->host_v6_routes);

            //num = p_rt_glb->current_remote_v6_routes;
            num = p_rt_glb->current_indirect_ipv6_route_num;
            SHOW_SYS_SPEC("Indirect IPv6 routes", num, p_sys_spec->indirect_v6_routes);

            //num = p_static_rt_cnt->rt_cnt_hash->count;
            num = p_rt_glb->current_static_v6_routes;
            SHOW_SYS_SPEC("Static IPv6 routes", num, p_sys_spec->static_v6_routes);

            //num = p_rt_glb->current_ecmp_v6_routes;
            //num = p_nhg_master->nhg_hash->count;
            num = p_rt_glb->current_ecmp_v6_groups;
            sal_sprintf(tmp_str, "IPv6 ecmp groups(member maximum %d)", p_sys_spec->ecmp_v6_member_maximum);
            SHOW_SYS_SPEC(tmp_str, num, p_sys_spec->ecmp_v6_routes);
        }
    }

/*begin add by taocy for BUG52801*/
#if defined(TSINGMA)
        sal_fprintf(fp,"number of IP NAT features:\n");
        SHOW_SYS_SPEC("Pools", p_nat_pool_master->nat_pool_hash->count, p_sys_spec->nat_pool_num);
        SHOW_SYS_SPEC("Rules", p_nat_rule_master->nat_rule_hash->count, p_sys_spec->nat_rule_num);
        SHOW_SYS_SPEC("Inside interfaces", p_nat_rtif_inside_master->nat_rtif_inside_hash->count, p_sys_spec->nat_inside_rtif_num);
        SHOW_SYS_SPEC("Outside interfaces", p_nat_rtif_outside_master->nat_rtif_outside_hash->count, p_sys_spec->nat_outside_rtif_num);
        SHOW_SYS_SPEC("Translations limit nodes", p_nat_session_limit_master->nat_session_limit_hash->count, p_sys_spec->nat_session_limit_node_num);
        SHOW_SYS_SPEC("Upstream translations", p_nat_global->session_cnt_upstream, p_sys_spec->nat_session_upstream_num);
        SHOW_SYS_SPEC("Dnstream translations", p_nat_global->session_cnt_dnstream, p_sys_spec->nat_session_dnstream_num);

        num = p_nat_global->session_fast_fwd_cnt_upstream + p_nat_global->session_fast_fwd_cnt_dnstream;
        SHOW_SYS_SPEC("Fast forward translations", num, p_sys_spec->nat_session_fast_path_entry_num);
#endif
/*end add by taocy for BUG52801*/

    sal_fprintf(fp,"number of Security features:\n");
    //num = p_acl_master->acl_entry_hash->count;
    //SHOW_SYS_SPEC("ACL entries", num, p_sys_spec->flow_entries_ingress + p_sys_spec->flow_entries_egress);
 #ifndef GREATBELT
    hsrv_sys_acl_entry_get_entry_dir_count(&ace_entry_ingress, &ace_entry_egress, &ace_entry_ingress6, &ace_entry_egress6, 
                            &ace_entry_wormfilter, &ace_entry_wormfilter6);
    if (GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
    {
        SHOW_SYS_SPEC_3INT_V6("IPv4+IPv6 ACL ingress entries", ace_entry_ingress+ace_entry_wormfilter, ace_entry_ingress6+ace_entry_wormfilter6, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC_3INT_V6("IPv4+IPv6 ACL egress entries", ace_entry_egress, ace_entry_egress6, p_sys_spec->flow_entries_egress);
    }
    else
    {
        SHOW_SYS_SPEC("MAC/IPv4 ACL ingress entries", ace_entry_ingress+ace_entry_wormfilter, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC("MAC/IPv4 ACL egress entries", ace_entry_egress, p_sys_spec->flow_entries_egress);
    }
 #else
    hsrv_sys_acl_entry_get_entry_dir_count_1(&entry_count);
    if (GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
    {
        SHOW_SYS_SPEC("IPv4 ACL ingress entries", entry_count.cur_ingress+entry_count.cur_worm_filter, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC("IPv4 ACL egress entries", entry_count.cur_egress, p_sys_spec->flow_entries_egress);
        SHOW_SYS_SPEC("IPv6 ACL ingress entries", entry_count.cur_ingress6+entry_count.cur_worm_filter6, p_sys_spec->flow6_entries_ingress);
        SHOW_SYS_SPEC("IPv6 ACL egress entries", entry_count.cur_egress6, p_sys_spec->flow6_entries_egress);
    }
    else
    {
        SHOW_SYS_SPEC("MAC/IPv4 ACL ingress entries", entry_count.cur_ingress+entry_count.cur_worm_filter, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC("MAC/IPv4 ACL egress entries", entry_count.cur_egress, p_sys_spec->flow_entries_egress);
    }
 #endif

    hsrv_acl_get_mac_ip_ipv6_acl_number(&ipacl_num, &macacl_num, &ipv6acl_num);
    hsrv_acl_get_mac_ip_ipv6_ace_number(&ipace_num, &macace_num, &ipv6ace_num);
    if (GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
    {
        SHOW_SYS_SPEC_4INT("System IP+MAC+IPV6 ACL configure", ipacl_num, macacl_num, ipv6acl_num, GLB_ACL_CONFIG_MAX);
        SHOW_SYS_SPEC_4INT("System IP+MAC+IPV6 ACE configure", ipace_num, macace_num, ipv6ace_num, GLB_ACE_CONFIG_MAX);
    }
    else
    {
        SHOW_SYS_SPEC_3INT("System IP+MAC ACL configure", ipacl_num, macacl_num, GLB_ACL_CONFIG_MAX);
        SHOW_SYS_SPEC_3INT("System IP+MAC ACE configure", ipace_num, macace_num, GLB_ACE_CONFIG_MAX);
    }
        
    num = hsrv_get_l4_port_num();
    SHOW_SYS_SPEC("System ACE L4 Port Range entries", num, GLB_ACL_L4PORT_ACE_NUM_MAX);

    num = p_sys_global->curr_vlanif_count;
    SHOW_SYS_SPEC_ONE("number of vlan interface", num, GLB_MAX_VLAN_IF_NUMBER);

    num = p_time_range_master->time_range_hash->count;
    SHOW_SYS_SPEC_ONE("number of time range", num, GLB_TIME_RANGE_MAX_NUM);

    num = p_db_glb->mac_user_count_current;
    SHOW_SYS_SPEC_ONE("number of dot1x mac based", num, p_sys_spec->dot1x_mac);

    sal_fprintf(fp,"number of L2 multicast features:\n");
    SHOW_SYS_SPEC("Group Member", p_igsp_glb->curr_group_member, p_igsp_glb->max_member_number);
    SHOW_SYS_SPEC("L2 Mcast Entry", p_mcfdb_master->mcfdb_hash->count, p_brg_glb->max_l2mcast_fdb);

    SHOW_SYS_SPEC_ONE("number of L3 multicast entries",  tbl_mrt_get_master()->mrt_hash->count, GLB_L3MC_MAX_ROUTE_ENTRY_NUM);
    num = hsrv_get_lag_num();
    SHOW_SYS_SPEC_ONE("number of link aggregation(static & lacp)",  num, p_sys_spec->lag_group_num);
    SHOW_SYS_SPEC_ONE("number of mac based vlan class ",  p_brg_glb->mac_based_vlan_class, p_sys_spec->mac_based_vlan_class);
    SHOW_SYS_SPEC_ONE("number of ipv4 based vlan class",  p_brg_glb->ipv4_based_vlan_class, p_sys_spec->ipv4_based_vlan_class);

    /*modified by yejl to fix bug44519, 2017-07-04*/
//  sal_fprintf(fp,"number of ipv6 based vlan class           : %d/%d\n", p_brg_glb->ipv6_based_vlan_class, p_sys_spec->ipv6_based_vlan_class);
    /*ended by yejl*/

    sal_fprintf(fp,"number of ARP ACL features:\n");
    SHOW_SYS_SPEC("System ARP ACL configure", p_arpacl_config_master->arpacl_config_hash->count, GLB_ACC_ARP_LIST_NUM_MAX);
    SHOW_SYS_SPEC("System ARP ACE configure", p_arpace_config_master->arpace_config_hash->count, GLB_ACC_ARP_ACE_NUM_MAX);

    sal_fprintf(fp,"number of OSPF features:\n");
    /* modified by liwh for bug 44506, 2017-06-28 */
    SHOW_SYS_SPEC("ospf network", p_ospf_master->ospf_network_list->count, GLB_OSPF_NETWORK_MAX);
    /* liwh end */
    SHOW_SYS_SPEC("ospf area range", p_ospf_area_range_master->ospf_area_range_hash->count, p_sys_spec->ospf_area_range_num);
    SHOW_SYS_SPEC("ospf area authentication", p_ospf_area_auth_master->ospf_area_auth_hash->count, OSPF_AREA_LIMIT_DEFAULT);

    sal_fprintf(fp,"number of qos features:\n");
#ifdef USW
    SHOW_SYS_SPEC("mpls label", mpls_label_num, GLB_QOS_MPLS_LABEL_NUM);
    SHOW_SYS_SPEC("policer action profile", policer_action_num, GLB_QOS_POLICER_ACTION_PROFILE_NUM);
    SHOW_SYS_SPEC("policer profile", policer_num, p_sys_spec->policer_profile_num);
    SHOW_SYS_SPEC("  ingress port policer profile", p_qos_master->in_port_policer_res_cnt, GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM);
    SHOW_SYS_SPEC("  egress port policer profile", p_qos_master->eg_port_policer_res_cnt, GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM);
    SHOW_SYS_SPEC("  ingress flow policer profile", p_qos_master->in_flow_policer_res_cnt, GLB_QOS_INGERSS_FLOW_POLICER_PROFILE_NUM);
    SHOW_SYS_SPEC("  egress flow policer profile", p_qos_master->eg_flow_policer_res_cnt, GLB_QOS_EGERSS_FLOW_POLICER_PROFILE_NUM);
#else
    SHOW_SYS_SPEC("policer profile", policer_num, p_sys_spec->policer_profile_num);
    if (policer_action_num);
    if (mpls_label_num);
    if (p_qos_master);
#endif
    SHOW_SYS_SPEC("drop profile", drop_num, p_sys_spec->drop_profile_num- 2);
    SHOW_SYS_SPEC("scheduler profile", queue_shape_num, p_sys_spec->queue_shape_profile_num - 1);
    SHOW_SYS_SPEC("port shape profile", port_shape_num, p_sys_spec->port_shape_profile_num - 1);

    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    SHOW_SYS_SPEC_ONE("number of vrrp sessions", p_db_vrrp_glb->vrrp_session_count, GLB_VRRP_MAX_SESSION);
    SHOW_SYS_SPEC_ONE("number of IP-SLA entries", p_db_ipsla_glb->ipsla_entry_num, GLB_IPSLA_ENTRY_MAX);
    SHOW_SYS_SPEC_ONE("number of track objects", p_db_track_glb->track_object_num, GLB_TRACK_OBJECT_MAX);

#ifdef TSINGMA    
    SHOW_SYS_SPEC_ONE("number of hash value", p_hash_value_pro_master->hash_value_profile_list->count, GLB_HASH_VALUE_PROFILE_MAX_NUM);
#endif

/* modified by liwh for bug 53010, 2019-07-22 */
#if defined (GOLDENGATE) || defined (GREATBELT)
    sal_fprintf(fp,"number of cfm features:\n");
    SHOW_SYS_SPEC("md", p_md_master->cfm_md_list->count, GLB_CFM_MD_MAX);
    SHOW_SYS_SPEC("maid", p_ma_master->cfm_ma_list->count, GLB_CFM_MA_MAX);
    SHOW_SYS_SPEC("local&remote mep", (p_lmep_master->cfm_lmep_list->count + p_rmep_master->cfm_rmep_list->count), GLB_CFM_LMEP_RMEP_MAX);
    sal_fprintf(fp,"number of g8032 features:\n");
    SHOW_SYS_SPEC("g8032 ring", p_g8032_master->g8032_ring_list->count, GLB_G8032_RING_MAX);
#endif
/* liwh end */    

    return PM_E_NONE;
}

int32
hsrv_show_tap_stm(tbl_sys_spec_t *p_sys_spec, FILE *fp, cfg_cmd_para_t *para, uint32 current, uint32 is_internal_debug)
{
    
    tbl_acl_entry_master_t      * p_acl_master      = tbl_acl_entry_get_master();
    tbl_acl_config_master_t     * p_acl_cfg_master  = tbl_acl_config_get_master();
    tbl_ace_config_master_t     * p_ace_cfg_master  = tbl_ace_config_get_master();
    tbl_sys_global_t            * p_sys_global      = tbl_sys_global_get_sys_global();
    tbl_tap_group_master_t      * p_tap_group_master  = tbl_tap_group_get_master();
#ifdef TSINGMA
    //tbl_hash_value_profile_master_t      * p_hash_value_pro_master = tbl_hash_value_profile_get_master();
#endif
    uint32 num = 0;

    acl_entry_count_t entry_count = {0};

    if (!p_acl_master || !p_sys_global || !p_tap_group_master)
    {
        return PM_E_NOT_INITED;
    }

	sal_fprintf(fp,"Current profile is                        : %s\n",
		p_sys_spec->key.type == GLB_STM_MODE_TAP_IPV4_IPV6 ? "flow6":"default");

    SHOW_SYS_SPEC_ONE("number of tap group", p_tap_group_master->tap_group_hash->count, GLB_TAP_GROUP_MAX);

    num = tbl_tap_group_get_truncation_count();
    SHOW_SYS_SPEC_ONE("number of tap trunction", num, GLB_TAP_TRUNCATION_MAX);

    num = hsrv_get_lag_num();
    SHOW_SYS_SPEC_ONE("number of link aggregation(static)", num, p_sys_spec->lag_group_num);

#ifdef TSINGMA    
    //SHOW_SYS_SPEC_ONE("number of hash value", p_hash_value_pro_master->hash_value_profile_list->count, GLB_HASH_VALUE_PROFILE_MAX_NUM);
#endif

    sal_fprintf(fp,"number of Flow features:\n");	

	tbl_acl_entry_get_entry_dir_count_1(&entry_count);

	if(check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
	{
		/* acl resource share */
		entry_count.cur_ingress = entry_count.cur_ingress + entry_count.cur_ingress6*2;
		entry_count.cur_egress = entry_count.cur_egress + entry_count.cur_egress6*2;
		SHOW_SYS_SPEC("Flow entry ingress entries", entry_count.cur_ingress, p_sys_spec->flow_entries_ingress);
	    SHOW_SYS_SPEC("Flow entry egress entries", entry_count.cur_egress, p_sys_spec->flow_entries_egress);
	}else{
		SHOW_SYS_SPEC("Flow entry ingress entries", entry_count.cur_ingress, p_sys_spec->flow_entries_ingress);
	    SHOW_SYS_SPEC("Flow entry egress entries", entry_count.cur_egress, p_sys_spec->flow_entries_egress);
		SHOW_SYS_SPEC("Flow6 entry ingress entries", entry_count.cur_ingress6, p_sys_spec->flow6_entries_ingress);
	    SHOW_SYS_SPEC("Flow6 entry egress entries", entry_count.cur_egress6, p_sys_spec->flow6_entries_egress);
	}
	
    num = p_acl_cfg_master->acl_config_hash->count;
    SHOW_SYS_SPEC("System Flow configure", num, GLB_ACL_CONFIG_MAX);
    num = p_ace_cfg_master->ace_config_hash->count;
    SHOW_SYS_SPEC("System Flow entry configure", num, GLB_ACE_CONFIG_MAX);

    num = hsrv_get_l4_port_num();
    SHOW_SYS_SPEC("System L4 Port Range entries", num, GLB_ACL_L4PORT_ACE_NUM_MAX);

    return PM_E_NONE;
}

int32
hsrv_show_hybrid_stm(tbl_sys_spec_t *p_sys_spec, FILE *fp, cfg_cmd_para_t *para, uint32 current, uint32 is_internal_debug)
{
    tbl_vlan_master_t           * p_vlan_master     = tbl_vlan_get_master();
    tbl_arp_master_t            * p_arp_master      = tbl_arp_get_master();
    tbl_iproute_node_master_t   * p_ipfib_master    = tbl_iproute_node_get_master();
    tbl_iproute_node_add_fail_count_master_t   * p_iproute_node_add_fail_count_master    = tbl_iproute_node_add_fail_count_get_master();
    tbl_route_global_t          * p_rt_glb          = tbl_route_global_get_route_global();
    tbl_acl_entry_master_t      * p_acl_master      = tbl_acl_entry_get_master();
    tbl_brg_global_t            * p_brg_glb         = tbl_brg_global_get_brg_global();
    tbl_ipsg_fib_master_t       * p_db_ipsg         = tbl_ipsg_fib_get_master();
    tbl_dot1x_global_t          * p_db_glb          = tbl_dot1x_global_get_dot1x_global();
    tbl_macfilter_master_t      * p_macfilter_master = tbl_macfilter_get_master();
    tbl_nexthop_group_master_t  * p_nhg              = tbl_nexthop_group_get_master();
    tbl_static_rt_cnt_master_t  * p_static_rt_cnt   = tbl_static_rt_cnt_get_master();
    tbl_time_range_master_t     * p_time_range_master = tbl_time_range_get_master();
    tbl_sys_global_t            * p_sys_global      = tbl_sys_global_get_sys_global();
    /* modified by liwh for bug 44583, 2017-07-05 */
    tbl_ospf_network_master_t   * p_ospf_master = tbl_ospf_network_get_master();
    /*liwh end */
    tbl_ospf_area_range_master_t   * p_ospf_area_range_master = tbl_ospf_area_range_get_master();
    tbl_ospf_area_auth_master_t *p_ospf_area_auth_master =  tbl_ospf_area_auth_get_master();
    tbl_ipsla_global_t          * p_db_ipsla_glb  = tbl_ipsla_global_get_ipsla_global();
    tbl_track_global_t          * p_db_track_glb  = tbl_track_global_get_track_global();
    tbl_qos_global_t  *p_qos_master = tbl_qos_global_get_qos_global();
    tbl_igsp_global_t *p_igsp_glb   = tbl_igsp_global_get_igsp_global();
    tbl_openflow_t    *p_openflow   = tbl_openflow_get_openflow();

/* modified by liwh for bug 53843, 2019-10-08 */
#if defined (GOLDENGATE) || defined (GREATBELT)
    tbl_cfm_ma_master_t      *p_ma_master = tbl_cfm_ma_get_master();
    tbl_cfm_lmep_master_t   *p_lmep_master = tbl_cfm_lmep_get_master();
    tbl_cfm_rmep_master_t   *p_rmep_master = tbl_cfm_rmep_get_master();
    tbl_cfm_md_master_t      *p_md_master = tbl_cfm_md_get_master();
    tbl_g8032_ring_master_t  *p_g8032_master = tbl_g8032_ring_get_master();
#endif
/* liwh end */

#ifdef TSINGMA
    tbl_hash_value_profile_master_t      * p_hash_value_pro_master = tbl_hash_value_profile_get_master();
#endif

    tbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    uint32 num = 0;
    uint32 fdb_static = 0;
    uint32 fdb_dynamic = 0;
    uint32 ucast_fdb = 0;
    uint32 security_dynamic = 0;
    uint32 security_static = 0;
    uint32 fdb_total = 0;
    uint32 drop_num = 0;
    uint32 queue_shape_num = 0;
    uint32 port_shape_num = 0;
    uint32 mpls_label_num = 0;
    #ifndef GREATBELT
    uint32 ace_entry_ingress = 0;
    uint32 ace_entry_egress = 0;
    uint32 ace_entry_ingress6 = 0;
    uint32 ace_entry_egress6 = 0;
    uint32 ace_entry_wormfilter = 0;
    uint32 ace_entry_wormfilter6 = 0;
    #endif
    uint32 policer_num = 0;
    uint32 policer_action_num = 0;
    uint32 ipacl_num = 0;
    uint32 macacl_num = 0;
    uint32 ipv6acl_num = 0;
    uint32 ipace_num = 0;
    uint32 macace_num = 0;
    uint32 ipv6ace_num = 0;
#ifdef GREATBELT
    acl_entry_count_t entry_count = {0};
 #endif
    char tmp_str[38]={0};

    drop_num = hsrv_get_qos_used_profile_num(1);
    queue_shape_num = hsrv_get_qos_used_profile_num(2);
    port_shape_num = hsrv_get_qos_used_profile_num(3);
    mpls_label_num = hsrv_get_qos_used_profile_num(5);
    policer_num = hsrv_get_qos_used_profile_num(0);
    policer_action_num = hsrv_get_qos_used_profile_num(4);

    if (!p_vlan_master || !p_arp_master || !p_acl_master || !p_sys_global
                || !p_db_ipsg || !p_db_glb || !p_nhg || !p_static_rt_cnt)
    {
        return PM_E_NOT_INITED;
    }

    if(GLB_STM_MODE_VLAN == p_sys_spec->key.type)
    {
        sal_fprintf(fp,"%s:\n", "vlan profile");
    }
    else if(GLB_STM_MODE_IPUCV6 == p_sys_spec->key.type)
    {
        sal_fprintf(fp,"%s:\n", "IPv6 profile");
    }
    else if (GLB_STM_MODE_OF_IPV6 == p_sys_spec->key.type)
    {
    #if 0
    #ifndef GOLDENGATE
        sal_fprintf(fp, "openflow ipv6 profile is not supported\n");
        return 0;
    #endif
    #endif
        sal_fprintf(fp,"%s:\n", "openflow IPv6 profile");
    }
    else
    {
        sal_fprintf(fp,"%s:\n", "default profile");
    }


    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_vlan_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        else
        {
            num ++;
        }
    }

    hsrv_fdb_get_count_number(&fdb_static, &security_static, &fdb_dynamic, &security_dynamic, &fdb_total);

    sal_fprintf(fp,"number of Ethernet features:\n");
//    SHOW_SYS_SPEC("MSTP instances", p_mstp_inst_glb->inst_list->count, p_sys_spec->mstp_instance);
    SHOW_SYS_SPEC("VLAN forwarding instances", num, p_sys_spec->vlan_instance);

    if (current && p_openflow->fdb_flow_valid)
    {
        /* add for hybrid fdb flow stm */
        ucast_fdb = p_sys_spec->ucast_fdb - GLB_OPENFLOW_FDB_FLOW_MAX_NUM;
    }
    else if (!current && p_openflow->fdb_flow_en)
    {
        ucast_fdb = p_sys_spec->ucast_fdb - GLB_OPENFLOW_FDB_FLOW_MAX_NUM;
    }
    else
    {
        ucast_fdb = p_sys_spec->ucast_fdb;
    }
    SHOW_SYS_SPEC("Ucast MAC addresses", fdb_total, ucast_fdb);
    SHOW_SYS_SPEC_SUB("static MAC address", fdb_static + security_static, p_brg_glb->max_static_fdb);
    SHOW_SYS_SPEC_SUB("dynamic MAC address", fdb_dynamic + security_dynamic, ucast_fdb);

    SHOW_SYS_SPEC("MAC Filter entry", p_macfilter_master->macflt_list->count, p_sys_spec->mac_filter);
    SHOW_SYS_SPEC("IGMP Host-join groups", p_igsp_glb->host_joined_num, IGMP_HOST_JOIN_GROUP_MAX);
    SHOW_SYS_SPEC("VLAN statistics", p_sys_global->vlan_stats_cnt, GLB_MAX_VLAN_STATS_ID);
    
    if (is_internal_debug)/*if interface is down, not consider*/
    {
        sal_fprintf(fp,"number of IP unicast features internal:\n");
        num = p_arp_master->arp_hash->count;
        num = p_rt_glb->current_host_routes;
        SHOW_SYS_SPEC("IPv4 host routes", num, p_sys_spec->host_routes);

        num = p_ipfib_master->rn_hash->count;
        num = p_rt_glb->current_remote_routes;
        SHOW_SYS_SPEC("Indirect IPv4 routes", num, p_sys_spec->indirect_routes);

        /* Static route belong to indirect routes but limit it to 1K */
        //num = p_static_rt_cnt->rt_cnt_hash->count;
        num = p_rt_glb->current_static_routes;
        /* For support all routes can be added by static route config */
        SHOW_SYS_SPEC("Static IPv4 routes", num, p_sys_spec->static_routes);

        num = p_rt_glb->current_ecmp_routes;
        //num = p_nhg->nhg_hash->count;
        sal_sprintf(tmp_str, "IPv4 ecmp groups(member maximum %d)", p_sys_spec->ecmp_member_maximum);
        SHOW_SYS_SPEC(tmp_str, num, p_sys_spec->ecmp_routes -GLB_OPENFLOW_SELECT_GROUP_MAX);

        num = p_db_ipsg->ipsg_fib_hash->count;
        //    SHOW_SYS_SPEC("IPv4 source guard entries", num, p_sys_spec->ipv4_source_guard);
    }
    else
    {
        sal_fprintf(fp,"number of IP unicast features:\n");
        num = p_arp_master->arp_hash->count;
        num = p_rt_glb->current_host_routes;
        SHOW_SYS_SPEC("IPv4 host routes", num, p_sys_spec->host_routes);

        num = p_ipfib_master->rn_hash->count;
        //num = p_rt_glb->current_remote_routes;
        num = p_rt_glb->current_indirect_ipv4_route_num;

        int32 frr_ospf_route_num = 0;
        frr_ospf_route_num = get_frr_ospf_route_num();
        if (frr_ospf_route_num != -1)
        {
            num += frr_ospf_route_num;
        }

        int32 route_add_fail_count = p_iproute_node_add_fail_count_master->add_fail_hash->count;
        if (num >= route_add_fail_count)
        {
            num -= route_add_fail_count;
        }
        
        SHOW_SYS_SPEC("Indirect IPv4 routes", num, p_sys_spec->indirect_routes);

        /* Static route belong to indirect routes but limit it to 1K */
        //num = p_static_rt_cnt->rt_cnt_hash->count;
        num = p_rt_glb->current_static_routes;
        /* For support all routes can be added by static route config */
        SHOW_SYS_SPEC("Static IPv4 routes", num, p_sys_spec->static_routes);

        //num = p_rt_glb->current_ecmp_routes;
        //num = p_nhg->nhg_hash->count;
        num = p_rt_glb->current_ecmp_groups;
        sal_sprintf(tmp_str, "IPv4 ecmp groups(member maximum %d)", p_sys_spec->ecmp_member_maximum);
        SHOW_SYS_SPEC(tmp_str, num, p_sys_spec->ecmp_routes -GLB_OPENFLOW_SELECT_GROUP_MAX);

        num = p_db_ipsg->ipsg_fib_hash->count;
        //    SHOW_SYS_SPEC("IPv4 source guard entries", num, p_sys_spec->ipv4_source_guard);
    }
    
    sal_fprintf(fp,"number of Security features:\n");
 #ifndef GREATBELT
    hsrv_sys_acl_entry_get_entry_dir_count(&ace_entry_ingress, &ace_entry_egress, &ace_entry_ingress6, &ace_entry_egress6, 
                            &ace_entry_wormfilter, &ace_entry_wormfilter6);
    if (GLB_STM_MODE_OF_IPV6 == p_sys_spec->key.type)
    {
        SHOW_SYS_SPEC_3INT_V6("IPv4+IPv6 ACL ingress entries", ace_entry_ingress+ace_entry_wormfilter, ace_entry_ingress6+ace_entry_wormfilter6, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC_3INT_V6("IPv4+IPv6 ACL egress entries", ace_entry_egress, ace_entry_egress6, p_sys_spec->flow_entries_egress);
    }
    else
    {
        SHOW_SYS_SPEC("MAC/IPv4 ACL ingress entries", ace_entry_ingress+ace_entry_wormfilter, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC("MAC/IPv4 ACL egress entries", ace_entry_egress, p_sys_spec->flow_entries_egress);
    }
 #else
    hsrv_sys_acl_entry_get_entry_dir_count_1(&entry_count);
    if (GLB_STM_MODE_OF_IPV6 == p_sys_spec->key.type)
    {
        SHOW_SYS_SPEC("IPv4 ACL ingress entries", entry_count.cur_ingress+entry_count.cur_worm_filter, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC("IPv4 ACL egress entries", entry_count.cur_egress, p_sys_spec->flow_entries_egress);
        SHOW_SYS_SPEC("IPv6 ACL ingress entries", entry_count.cur_ingress6+entry_count.cur_worm_filter6, p_sys_spec->flow6_entries_ingress);
        SHOW_SYS_SPEC("IPv6 ACL egress entries", entry_count.cur_egress6, p_sys_spec->flow6_entries_egress);
    }
    else
    {
        SHOW_SYS_SPEC("MAC/IPv4 ACL ingress entries", entry_count.cur_ingress+entry_count.cur_worm_filter, p_sys_spec->flow_entries_ingress);
        SHOW_SYS_SPEC("MAC/IPv4 ACL egress entries", entry_count.cur_egress, p_sys_spec->flow_entries_egress);
    }
 #endif

    hsrv_acl_get_mac_ip_ipv6_acl_number(&ipacl_num, &macacl_num, &ipv6acl_num);
    hsrv_acl_get_mac_ip_ipv6_ace_number(&ipace_num, &macace_num, &ipv6ace_num);
    if (GLB_STM_MODE_OF_IPV6 == p_sys_spec->key.type)
    {
        SHOW_SYS_SPEC_4INT("System IP+MAC+IPV6 ACL configure", ipacl_num, macacl_num, ipv6acl_num, GLB_ACL_CONFIG_MAX);
        SHOW_SYS_SPEC_4INT("System IP+MAC+IPV6 ACE configure", ipace_num, macace_num, ipv6ace_num, GLB_ACE_CONFIG_MAX);
    }
    else
    {
        SHOW_SYS_SPEC_3INT("System IP+MAC ACL configure", ipacl_num, macacl_num, GLB_ACL_CONFIG_MAX);
        SHOW_SYS_SPEC_3INT("System IP+MAC ACE configure", ipace_num, macace_num, GLB_ACE_CONFIG_MAX);
    }
    num = hsrv_get_l4_port_num();
    SHOW_SYS_SPEC("System ACE L4 Port Range entries", num, GLB_ACL_L4PORT_ACE_NUM_MAX);
    /* add by wangyb for bug 44396, 2017-10-19 */
    num = hsrv_get_lag_num();
    SHOW_SYS_SPEC_ONE("number of link aggregation(static & lacp)", num, p_sys_spec->lag_group_num);
    
    num = p_sys_global->curr_vlanif_count;
    SHOW_SYS_SPEC_ONE("number of vlan interface", num, GLB_MAX_VLAN_IF_NUMBER);

    num = p_time_range_master->time_range_hash->count;
    SHOW_SYS_SPEC_ONE("number of time range", num, GLB_TIME_RANGE_MAX_NUM);/*added by yejl for bug 49723, 2018-11-27*/

    num = p_db_glb->mac_user_count_current;
//    sal_fprintf(fp,"number of dot1x mac based                 : %d/%d\n", num, p_sys_spec->dot1x_mac);

//    sal_fprintf(fp,"number of L2 multicast features:\n");
//    SHOW_SYS_SPEC("Group Member", p_igsp_glb->curr_group_member, p_igsp_glb->max_member_number);
//    SHOW_SYS_SPEC("L2 Mcast Entry", p_mcfdb_master->mcfdb_hash->count, p_brg_glb->max_l2mcast_fdb);

//    sal_fprintf(fp,"number of mac based vlan class            : %d/%d\n", p_brg_glb->mac_based_vlan_class, p_sys_spec->mac_based_vlan_class);
//    sal_fprintf(fp,"number of ipv4 based vlan class           : %d/%d\n", p_brg_glb->ipv4_based_vlan_class, p_sys_spec->ipv4_based_vlan_class);
//    sal_fprintf(fp,"number of ipv6 based vlan class           : %d/%d\n", p_brg_glb->ipv6_based_vlan_class, p_sys_spec->ipv6_based_vlan_class);

    sal_fprintf(fp,"number of OSPF features:\n");
    /* modified by liwh for bug 44583, 2017-06-28 */
    SHOW_SYS_SPEC("ospf network", p_ospf_master->ospf_network_list->count, GLB_OSPF_NETWORK_MAX);
    /* liwh end */
    SHOW_SYS_SPEC("ospf area range", p_ospf_area_range_master->ospf_area_range_hash->count, p_sys_spec->ospf_area_range_num);
    SHOW_SYS_SPEC("ospf area authentication", p_ospf_area_auth_master->ospf_area_auth_hash->count, OSPF_AREA_LIMIT_DEFAULT);
    
    /*for protected vlan*/
    SHOW_SYS_SPEC_ONE("number of protected vlan", p_sys_global->protected_vlan_cnt, OPENFLOW_PROTECTED_VLAN);
    /* liwh end */

    sal_fprintf(fp,"number of qos features:\n");
#ifdef USW
    SHOW_SYS_SPEC("mpls label", mpls_label_num, GLB_QOS_MPLS_LABEL_NUM);
    SHOW_SYS_SPEC("policer action profile", policer_action_num, GLB_QOS_POLICER_ACTION_PROFILE_NUM);
    SHOW_SYS_SPEC("policer profile", policer_num, p_sys_spec->policer_profile_num);/*added by yejl for bug 52691, 2019-06-25*/
    SHOW_SYS_SPEC("  ingress port policer profile", p_qos_master->in_port_policer_res_cnt, GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM);
    SHOW_SYS_SPEC("  egress port policer profile", p_qos_master->eg_port_policer_res_cnt, GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM);
#else
    SHOW_SYS_SPEC("policer profile", policer_num, p_sys_spec->policer_profile_num);
    if (mpls_label_num);
    if (policer_action_num);
    if (p_qos_master);
#endif
    SHOW_SYS_SPEC("drop profile", drop_num, p_sys_spec->drop_profile_num- 2);
    SHOW_SYS_SPEC("scheduler profile", queue_shape_num, p_sys_spec->queue_shape_profile_num - 1);
    SHOW_SYS_SPEC("port shape profile", port_shape_num, p_sys_spec->port_shape_profile_num - 1);

    SHOW_SYS_SPEC_ONE("number of IP-SLA entries", p_db_ipsla_glb->ipsla_entry_num, GLB_IPSLA_ENTRY_MAX);
    SHOW_SYS_SPEC_ONE("number of track objects", p_db_track_glb->track_object_num, GLB_TRACK_OBJECT_MAX);

/* modified by liwh for bug 53843, 2019-10-08 */
#if defined (GOLDENGATE) || defined (GREATBELT)
    sal_fprintf(fp,"number of cfm features:\n");
    SHOW_SYS_SPEC("md", p_md_master->cfm_md_list->count, GLB_CFM_MD_MAX);
    SHOW_SYS_SPEC("maid", p_ma_master->cfm_ma_list->count, GLB_CFM_MA_MAX);
    SHOW_SYS_SPEC("local&remote mep", (p_lmep_master->cfm_lmep_list->count + p_rmep_master->cfm_rmep_list->count), GLB_CFM_LMEP_RMEP_MAX);
    sal_fprintf(fp,"number of g8032 features:\n");
    SHOW_SYS_SPEC("g8032 ring", p_g8032_master->g8032_ring_list->count, GLB_G8032_RING_MAX);
#endif
/* liwh end */    

#ifdef TSINGMA
    SHOW_SYS_SPEC_ONE("number of hash value", p_hash_value_pro_master->hash_value_profile_list->count, GLB_HASH_VALUE_PROFILE_MAX_NUM);
#endif
    return PM_E_NONE;
}

int32
hsrv_cmd_show_stm(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;
    tbl_sys_spec_t * p_sys_spec = NULL;
    int32 type = 1000;
    uint32 current = 0;
    uint32 is_internal_debug = FALSE;

    switch (para->oper)
    {
    case CDB_OPER_GET:

 /*mod by hansf for support show next profile*/
 #if 0
    #ifdef OFPRODUCT
        if(argc > 0)
        {
            if (0 == sal_strcmp(argv[0], "next"))
            {
                CFG_INVALID_PARAM_RET("Show next profile is not supported in hybrid system");
            }
        }
    #endif
#endif

        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }
        hsrv_cmd_map_sys_spec_type(argv, argc, para, &type, &current);

        if (argc == 2 && current == TRUE && 0 == sal_memcmp(argv[1], "internal_debug", sizeof("internal_debug")))
        {
            is_internal_debug = TRUE;
        }

        if (type < GLB_STM_MODE_MAX)
        {
            p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
            if(p_sys_spec)
            {
#ifdef TAPPRODUCT
                PM_E_RETURN(hsrv_show_tap_stm(p_sys_spec, fp, para, current, is_internal_debug));
#else
    #ifdef OFPRODUCT
                PM_E_RETURN(hsrv_show_hybrid_stm(p_sys_spec, fp, para, current, is_internal_debug));
    #else
                PM_E_RETURN(hsrv_show_stm(p_sys_spec, fp, para, current, is_internal_debug));
    #endif
#endif
            }
        }

        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}
