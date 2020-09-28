#include "proto.h"
#include "glb_acl_define.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h" 
#include "gen/tbl_class_in_policy_config_define.h" 
#include "gen/tbl_class_in_policy_config.h" 
#include "gen/tbl_acl_in_class_config_define.h" 
#include "gen/tbl_acl_in_class_config.h"
#include "gen/tbl_acl_config_define.h" 
#include "gen/tbl_acl_config.h"
#include "gen/tbl_ace_config_define.h" 
#include "gen/tbl_ace_config.h"
#include "gen/tbl_acl_entry_action_define.h" 
#include "gen/tbl_acl_entry_action.h"
#include "gen/tbl_acl_entry_define.h" 
#include "gen/tbl_acl_entry.h"
#include "gen/tbl_acl_udf_entry_define.h"
#include "gen/tbl_acl_udf_entry.h"
#include "gen/tbl_time_range_define.h"
#include "gen/tbl_time_range.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_class_map_action_config_define.h" 
#include "gen/tbl_class_map_action_config.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_qos_policer_profile_define.h" 
#include "gen/tbl_qos_policer_profile.h"
#include "gen/tbl_fea_acl_define.h" 
#include "gen/tbl_fea_acl.h"
#include "gen/tbl_fea_acl_policy_action_define.h" 
#include "gen/tbl_fea_acl_policy_action.h"
#include "gen/tbl_sys_spec_define.h"
#include "switch.h"
#include "acl.h"
#include "acl_policy.h"
#include "if_agg.h"


int32
_acl_policy_first_get_ipv4_ipv6_ace_number(tbl_acl_config_t *pst_acl_config, uint32 ingress_en, uint32 ethertype_or_arp_flag, uint32* ace_count_ipv4, uint32* ace_count_ipv6)
{
    uint32 p_op_ace_count_ipv4 = 0;
    uint32 p_op_ace_count_ipv6 = 0;

    if (pst_acl_config->type_identifying == GLB_ACL_CONFIG_TYPE_IPV6)
    {
        p_op_ace_count_ipv6 ++;
        if (ingress_en)
        {
            *ace_count_ipv6 = p_op_ace_count_ipv6 + pst_acl_config->intf_ref_in;
        }
        else
        {
            *ace_count_ipv6 = p_op_ace_count_ipv6 + pst_acl_config->intf_ref_out;
        }
    }
    else if (pst_acl_config->type_identifying == GLB_ACL_CONFIG_TYPE_MAC)
    {
        if (TRUE == acl_stm_is_ipv6_profile())
        {   
            if (ethertype_or_arp_flag == 0)
            {
                p_op_ace_count_ipv4 ++; 
                p_op_ace_count_ipv6 ++;
                if (ingress_en)
                {
                    *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_in;
                    *ace_count_ipv6 = p_op_ace_count_ipv6 + pst_acl_config->intf_ref_in;
                }
                else
                {
                    *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_out;
                    *ace_count_ipv6 = p_op_ace_count_ipv6 + pst_acl_config->intf_ref_out;
                }
            }
            else
            {
                p_op_ace_count_ipv4 ++;
                if (ingress_en)
                {
                    *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_in;
                }
                else
                {
                    *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_out;
                }
            }
        }
        else
        {
            p_op_ace_count_ipv4 ++;
            if (ingress_en)
            {
                *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_in;
            }
            else
            {
                *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_out;
            }
        }
    }
    else if (pst_acl_config->type_identifying == GLB_ACL_CONFIG_TYPE_NORMAL)
    {
        p_op_ace_count_ipv4 ++;
        if (ingress_en)
        {
            *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_in;
        }
        else
        {
            *ace_count_ipv4 = p_op_ace_count_ipv4 + pst_acl_config->intf_ref_out;
        }
    }
    return PM_E_NONE;
}

int32
_acl_policy_not_first_get_ipv4_ipv6_ace_number(tbl_acl_config_t *pst_acl_config, uint32 ethertype_or_arp_flag, uint32* ace_count_ipv4, uint32* ace_count_ipv6)
{
    uint32 p_op_ace_count_ipv4 = 0;
    uint32 p_op_ace_count_ipv6 = 0;
    
    if (pst_acl_config->type_identifying == GLB_ACL_CONFIG_TYPE_IPV6)
    {
        p_op_ace_count_ipv6 ++;
    }
    else if (pst_acl_config->type_identifying == GLB_ACL_CONFIG_TYPE_MAC)
    {
        if (TRUE == acl_stm_is_ipv6_profile())
        {   
            if (ethertype_or_arp_flag == 0)
            {
                p_op_ace_count_ipv4 ++; 
                p_op_ace_count_ipv6 ++;
            }
            else
            {
                p_op_ace_count_ipv4 ++;
            }
        }
        else
        {
            p_op_ace_count_ipv4 ++;
        }
    }
    else if (pst_acl_config->type_identifying == GLB_ACL_CONFIG_TYPE_NORMAL)
    {
        p_op_ace_count_ipv4 ++;
    }
    *ace_count_ipv4 = p_op_ace_count_ipv4;
    *ace_count_ipv6 = p_op_ace_count_ipv6;
    return PM_E_NONE;
}

int32
_acl_entry_attach_interface_by_policy_res_check(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    char* acl_name               = pargs->argv[0];
    uint32 *p_op_ace_count_ipv4  = pargs->argv[1];
    uint32 *p_op_ace_count_ipv6  = pargs->argv[2];

    if (0 == sal_strcmp(p_ace_config->key.acl_name, acl_name))
    {
        if (p_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            if (TRUE == acl_stm_is_ipv6_profile())
            {   
                if (!GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_MAC_ACE_ETHTYPE) &&
                    !(GLB_FLAG_ISSET(p_ace_config->key_arp_flags, GLB_ARP_ACE_OP_CODE)))
                {
                    (*p_op_ace_count_ipv4) ++; 
                    (*p_op_ace_count_ipv6) ++;
                }
                else
                {
                    (*p_op_ace_count_ipv4) ++;
                }
            }
            else
            {
                (*p_op_ace_count_ipv4) ++;
            }
        }
        else if (p_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
        {
             (*p_op_ace_count_ipv4) ++;
        }
        else if (p_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            (*p_op_ace_count_ipv6) ++;
        }
    }

    return PM_E_NONE;
}

int32
acl_entry_attach_interface_by_policy_res_check(char *acl_name, uint32* p_op_ace_count_ipv4, uint32* p_op_ace_count_ipv6)
{
    tbl_iter_args_t args;

    args.argv[0] = acl_name;
    args.argv[1] = p_op_ace_count_ipv4;
    args.argv[2] = p_op_ace_count_ipv6;

    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_entry_attach_interface_by_policy_res_check, &args);
    return PM_E_NONE;
}



int32
acl_policy_operation_if_pmap_add_res_check(char* interface_name, uint32 direct_in, char* policy_map_name)
{
    tbl_interface_t *pst_interface = NULL;
    tbl_policy_map_config_t* pst_policy_map_config = NULL;
    tbl_class_in_policy_config_master_t *p_class_in_policy_master = tbl_class_in_policy_config_get_master();
    ctclib_slistnode_t *class_in_policy_listnode = NULL;
    ctclib_slistnode_t *class_in_policy_next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    uint32 tcp_flag_count1 = 0;
    uint32 tcp_flag_count = 0;
    uint32 stm_max = 0;

    uint32 intf_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 policy_tcp_flags_count = acl_policy_get_tcp_flags_config_count();
    
    uint32 current_acl_entry_count_ingress = 0;
    uint32 current_acl_entry_count_egress = 0;
    uint32 current_acl_entry_count_wromfilter = 0;
    uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;
    acl_entry_count_t entry_count = {0};
    int32  share_resource = TRUE;

    share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);
    if(share_resource)
	{
		acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}
    else
	{
		acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}

    if(direct_in)
    {
        /* modified by huwx for bug 39772, acl stats id only support 1-4095 */
        stm_max = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    }
    else
    {
        stm_max = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
    }

    uint32 op_ace_count_ipv4 = 0;
    uint32 op_ace_count_ipv6 = 0;

    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_NOT_EXIST;
    }
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config_by_name(policy_map_name);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_NOT_EXIST;
    }

    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, policy_map_name))
        {
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
                    sal_strcpy(acl_config_key.name, p_db_acl_in_class_config->key.acl_name);
                    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
                    if (NULL != pst_acl_config)
                    {
                        tcp_flag_count1 = pst_acl_config->tcp_flags_ref;
                        tcp_flag_count = tcp_flag_count+ tcp_flag_count1;
                    }
                    /*end*/
                    acl_entry_attach_interface_by_policy_res_check(p_db_acl_in_class_config->key.acl_name, &op_ace_count_ipv4, &op_ace_count_ipv6);
                }
            }
        }
    }

    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
    if (tcp_flag_count + intf_tcp_flags_count + policy_tcp_flags_count > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }
    /*end*/
    
    if(direct_in)
    {
        if(share_resource)
        {
            if((current_acl_entry_count_ingress + current_acl_entry_count_wromfilter + op_ace_count_ipv4 + op_ace_count_ipv6 *2) > stm_max)
            {
                return PM_E_CFG_NO_RESOURCE;
            }
        }
        else
        {
            if((entry_count.cur_ingress + entry_count.cur_worm_filter + op_ace_count_ipv4) > stm_max)
            {
                return PM_E_CFG_NO_RESOURCE;
            }
            if ((entry_count.cur_ingress6 + entry_count.cur_worm_filter6 + op_ace_count_ipv6) > stm_max_ingress6)
            {
                return PM_E_CFG_NO_RESOURCE;
            }
        }
        
    }
    else
    {
        if(share_resource)
        {
            if((current_acl_entry_count_egress + op_ace_count_ipv4 + op_ace_count_ipv6 *2) > stm_max)
            {
                return PM_E_CFG_NO_RESOURCE;
            }
        }
        else
        {
            if((entry_count.cur_egress + op_ace_count_ipv4) > stm_max)
            {
                return PM_E_CFG_NO_RESOURCE;
            }
            if ((entry_count.cur_egress6 + op_ace_count_ipv6) > stm_max_egress6)
            {
                return PM_E_CFG_NO_RESOURCE;
            }
        }
        
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_pmap_class_add_res_check(char*policy_map_name, char*class_map_name)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    uint32 tcp_flag_count1 = 0;
    uint32 tcp_flag_count = 0;

    uint32 intf_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 policy_tcp_flags_count = acl_policy_get_tcp_flags_config_count();
    
    uint32 stm_max_ingress = 0;
    uint32 stm_max_egress = 0;
    uint32 current_acl_entry_count_ingress = 0;
    uint32 current_acl_entry_count_egress = 0;
    uint32 current_acl_entry_count_wromfilter = 0;
    uint32 op_ace_count_ingress_ipv4 = 0;
    uint32 op_ace_count_egress_ipv4 = 0;
    uint32 op_ace_count_ingress_ipv6 = 0;
    uint32 op_ace_count_egress_ipv6 = 0;
    uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;
    int32  share_resource = TRUE;
    acl_entry_count_t entry_count = {0};

    share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);
    if(share_resource)
	{
		acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}
    else
	{
		acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}

    /* modified by huwx for bug 39772, acl stats id only support 1-4095 */
    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
    
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if (0 == sal_strcmp(p_db_if->igs_policy_map, policy_map_name))
        {
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            if (NULL != p_db_class_in_policy_config)
            {
                CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
                {
                    if (0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                    {
                        /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                        sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
                        sal_strcpy(acl_config_key.name, p_db_acl_in_class_config->key.acl_name);
                        pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
                        if (NULL != pst_acl_config)
                        {
                            tcp_flag_count1 = pst_acl_config->tcp_flags_ref;
                            tcp_flag_count = tcp_flag_count + tcp_flag_count1;
                        }
                        /*end*/
                        acl_entry_attach_interface_by_policy_res_check(p_db_acl_in_class_config->key.acl_name, &op_ace_count_ingress_ipv4, &op_ace_count_ingress_ipv6);
                    }
                }
            }
        }

        if (0 == sal_strcmp(p_db_if->egs_policy_map, policy_map_name))
        {
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            if (NULL != p_db_class_in_policy_config)
            {
                CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
                {
                    if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                    {
                        /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                        sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
                        sal_strcpy(acl_config_key.name, p_db_acl_in_class_config->key.acl_name);
                        pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
                        if (NULL != pst_acl_config)
                        {
                            tcp_flag_count1 = pst_acl_config->tcp_flags_ref;
                            tcp_flag_count = tcp_flag_count + tcp_flag_count1;
                        }
                        /*end*/
                        acl_entry_attach_interface_by_policy_res_check(p_db_acl_in_class_config->key.acl_name, &op_ace_count_egress_ipv4, &op_ace_count_egress_ipv6);
                    }
                }
            }
        }
    }

    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
    if (tcp_flag_count + intf_tcp_flags_count + policy_tcp_flags_count > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }
    /*end*/
    
    if(share_resource)
    {
        if (((current_acl_entry_count_ingress + current_acl_entry_count_wromfilter + op_ace_count_ingress_ipv4 + op_ace_count_ingress_ipv6 *2) > stm_max_ingress) || 
            ((current_acl_entry_count_egress + op_ace_count_egress_ipv4 + op_ace_count_egress_ipv6 *2) > stm_max_egress))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
    }
    else
    {
        if(((entry_count.cur_ingress + entry_count.cur_worm_filter + op_ace_count_ingress_ipv4) > stm_max_ingress) ||
            ((entry_count.cur_egress + op_ace_count_egress_ipv4) > stm_max_egress))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
        if(((entry_count.cur_ingress6 + entry_count.cur_worm_filter6 + op_ace_count_ingress_ipv6) > stm_max_ingress6) ||
            ((entry_count.cur_egress6 + op_ace_count_egress_ipv6) > stm_max_egress6))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
    }
    

    return PM_E_NONE;
}

int32
acl_policy_operation_class_acl_add_res_check(char *class_map_name, char *acl_name)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    uint32 tcp_flag_count1 = 0;
    uint32 tcp_flag_count = 0;

    uint32 intf_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 policy_tcp_flags_count = acl_policy_get_tcp_flags_config_count();
    
    uint32 stm_max_ingress = 0;
    uint32 stm_max_egress = 0;
    uint32 current_acl_entry_count_ingress = 0;
    uint32 current_acl_entry_count_egress = 0;
    uint32 current_acl_entry_count_wromfilter = 0;
    uint32 op_ace_count_ingress_ipv4 = 0;
    uint32 op_ace_count_egress_ipv4 = 0;
    uint32 op_ace_count_ingress_ipv6 = 0;
    uint32 op_ace_count_egress_ipv6 = 0;
    uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;
    int32  share_resource = TRUE;
    acl_entry_count_t entry_count = {0};
    
    share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);
    if(share_resource)
	{
		acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}
    else
	{
		acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}
    /* modified by huwx for bug 39772, acl stats id only support 1-4095 */
    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if (0 != sal_strlen(p_db_if->igs_policy_map))
        {
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->igs_policy_map, class_map_name);
            if(NULL != p_db_class_in_policy_config)
            {
                p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(class_map_name, acl_name);
                if (NULL != p_db_acl_in_class_config)
                {
                    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
                    sal_strcpy(acl_config_key.name, p_db_acl_in_class_config->key.acl_name);
                    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
                    if (NULL != pst_acl_config)
                    {
                        tcp_flag_count1 = pst_acl_config->tcp_flags_ref;
                        tcp_flag_count = tcp_flag_count + tcp_flag_count1;
                    }
                    /*end*/
                    acl_entry_attach_interface_by_policy_res_check(p_db_acl_in_class_config->key.acl_name, &op_ace_count_ingress_ipv4, &op_ace_count_ingress_ipv6);
                }
            }
        }

        if (0 != sal_strlen(p_db_if->egs_policy_map))
        {
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->egs_policy_map, class_map_name);
            if(NULL != p_db_class_in_policy_config)
            {
                p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(class_map_name, acl_name);
                if (NULL != p_db_acl_in_class_config)
                {
                    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
                    sal_strcpy(acl_config_key.name, p_db_acl_in_class_config->key.acl_name);
                    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
                    if (NULL != pst_acl_config)
                    {
                        tcp_flag_count1 = pst_acl_config->tcp_flags_ref;
                        tcp_flag_count = tcp_flag_count + tcp_flag_count1;
                    }
                    /*end*/
                    acl_entry_attach_interface_by_policy_res_check(p_db_acl_in_class_config->key.acl_name, &op_ace_count_egress_ipv4, &op_ace_count_egress_ipv6);
                }
            }
        }
    }

    /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
    if (tcp_flag_count + intf_tcp_flags_count + policy_tcp_flags_count > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
    {
        return PM_E_CFG_ACL_TCP_NO_RESPURCE;
    }
    /*end*/
    
    if(share_resource)
    {
        if (((current_acl_entry_count_ingress + current_acl_entry_count_wromfilter + op_ace_count_ingress_ipv4 + op_ace_count_ingress_ipv6 *2) > stm_max_ingress) ||
            ((current_acl_entry_count_egress + op_ace_count_egress_ipv4 + op_ace_count_egress_ipv6 *2) > stm_max_egress))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
    }
    else
    {
        if(((entry_count.cur_ingress + entry_count.cur_worm_filter + op_ace_count_ingress_ipv4) > stm_max_ingress) ||
            ((entry_count.cur_egress + op_ace_count_egress_ipv4) > stm_max_egress))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
        if(((entry_count.cur_ingress6 + entry_count.cur_worm_filter6 + op_ace_count_ingress_ipv6) > stm_max_ingress6) ||
            ((entry_count.cur_egress6 + op_ace_count_egress_ipv6) > stm_max_egress6))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_acl_ace_add_res_check(char *acl_name, uint32 ethertype_or_arp_flag, uint32 tcp_flags)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;

    tbl_class_in_policy_config_master_t *p_class_in_policy_master = tbl_class_in_policy_config_get_master();
    ctclib_slistnode_t *class_in_policy_listnode = NULL;
    ctclib_slistnode_t *class_in_policy_next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;

    uint32 intf_tcp_flags_count = tbl_acl_config_get_tcp_flags_config_count();
    uint32 policy_tcp_flags_count = acl_policy_get_tcp_flags_config_count();
    uint32 tcp_flag_count = 0;
    uint32 tcp_flag_count_intf = 0;
    
    uint32 stm_max_ingress = 0;
    uint32 stm_max_egress = 0;
    uint32 current_acl_entry_count_ingress = 0;
    uint32 current_acl_entry_count_egress = 0;
    uint32 current_acl_entry_count_wromfilter = 0;

    uint32 ace_count_ingress_ipv4 = 0;
    uint32 ace_count_egress_ipv4 = 0;
    uint32 ace_count_ingress_ipv6 = 0;
    uint32 ace_count_egress_ipv6 = 0;
    
    uint32 op_ace_count_ingress_ipv4 = 0;
    uint32 op_ace_count_egress_ipv4 = 0;
    uint32 op_ace_count_ingress_ipv6 = 0;
    uint32 op_ace_count_egress_ipv6 = 0;
    uint32 stm_max_ingress6 = 0;
    uint32 stm_max_egress6 = 0;
    int32  share_resource = FALSE;
    uint32 first_research_in = TRUE;
    uint32 first_research_out = TRUE;
    acl_entry_count_t entry_count = {0};

    share_resource = check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE);
    if(share_resource)
	{
		acl_entry_get_entry_dir_count(&current_acl_entry_count_ingress, 
			                                 &current_acl_entry_count_egress, 
			                                 &current_acl_entry_count_wromfilter);
	}
    else
	{
		acl_entry_get_entry_dir_count_1(&entry_count);
		stm_max_ingress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS);
    	stm_max_egress6 = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS);
	}
    /* modified by huwx for bug 39772, acl stats id only support 1-4095 */
    stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);

    /*modified by yejl for bug 49719, 2018-10-31*/
    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
    sal_strcpy(acl_config_key.name, acl_name);
    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        return PM_E_NONE;
    }
    else
    {
        tcp_flag_count_intf = (pst_acl_config->intf_ref_in + pst_acl_config->intf_ref_out) * tcp_flags;
    }
    /*end*/
                            
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if (0 != sal_strlen(p_db_if->igs_policy_map))
        {
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if(NULL != p_db_class_in_policy_config)
                {
                    if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->igs_policy_map))
                    {
                        p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, acl_name);
                        if(NULL != p_db_acl_in_class_config)
                        {
                            /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                            if (tcp_flags)
                            {
                                tcp_flag_count++;
                            }
                            /*end*/
                            
                            /*modified by yejl for bug 49719, 2018-10-31*/
                            if (first_research_in)
                            {
                                _acl_policy_first_get_ipv4_ipv6_ace_number(pst_acl_config, TRUE, ethertype_or_arp_flag, &ace_count_ingress_ipv4, &ace_count_ingress_ipv6);
                                first_research_in = FALSE;
                            }
                            else
                            {
                                _acl_policy_not_first_get_ipv4_ipv6_ace_number(pst_acl_config, ethertype_or_arp_flag, &ace_count_ingress_ipv4, &ace_count_ingress_ipv6);
                            }
                            op_ace_count_ingress_ipv4 = op_ace_count_ingress_ipv4 + ace_count_ingress_ipv4;
                            op_ace_count_ingress_ipv6 = op_ace_count_ingress_ipv6 + ace_count_ingress_ipv6;
                            /*end*/
                        }
                    }
                }
            }
        }

        if (0 != sal_strlen(p_db_if->egs_policy_map))
        {
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if(NULL != p_db_class_in_policy_config)
                {
                    if(0 ==sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->egs_policy_map))
                    {
                        p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, acl_name);
                        if(NULL != p_db_acl_in_class_config)
                        {
                            /*modified by yejl for bug 49872 and bug 49884, 2018-11-08*/
                            if (tcp_flags)
                            {
                                tcp_flag_count++;
                            }
                            /*end*/
                            
                            /*modified by yejl for bug 49719, 2018-10-31*/
                            if (first_research_out)
                            {
                                _acl_policy_first_get_ipv4_ipv6_ace_number(pst_acl_config, FALSE, ethertype_or_arp_flag, &ace_count_egress_ipv4, &ace_count_egress_ipv6);
                                first_research_out = FALSE;
                            }
                            else
                            {
                                _acl_policy_not_first_get_ipv4_ipv6_ace_number(pst_acl_config, ethertype_or_arp_flag, &ace_count_egress_ipv4, &ace_count_egress_ipv6);
                            }
                            op_ace_count_egress_ipv4 = op_ace_count_egress_ipv4 + ace_count_egress_ipv4;
                            op_ace_count_egress_ipv6 = op_ace_count_egress_ipv6 + ace_count_egress_ipv6;
                            /*end*/

                            /*modified by yejl for bug 49782, 2018-11-01*/
                            if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
                            {
                                if (ethertype_or_arp_flag == 1)
                                {
                                    return PM_E_CFG_PM_ERROR;
                                }
                            }
                            /*end*/
                        }
                    }
                }
            }
        }
    }

    /*modified by yejl for bug 49872 and bug 49884 and bug 49920, 2018-11-08*/
    if (tcp_flags)
    {
        if (tcp_flag_count + tcp_flag_count_intf + intf_tcp_flags_count + policy_tcp_flags_count > GLB_ACL_TCP_FLAGS_ACE_NUM_MAX)
        {
            return PM_E_CFG_ACL_TCP_NO_RESPURCE;
        }
    }
    /*end*/
    
    if(share_resource)
    {
        if (((current_acl_entry_count_ingress + current_acl_entry_count_wromfilter + op_ace_count_ingress_ipv4 + op_ace_count_ingress_ipv6 *2) > stm_max_ingress) ||
            ((current_acl_entry_count_egress + op_ace_count_egress_ipv4 + op_ace_count_egress_ipv6 *2) > stm_max_egress))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
    }
    else
    {
        if(((entry_count.cur_ingress + entry_count.cur_worm_filter + op_ace_count_ingress_ipv4) > stm_max_ingress) ||
            ((entry_count.cur_egress + op_ace_count_egress_ipv4) > stm_max_egress))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
        if(((entry_count.cur_ingress6 + entry_count.cur_worm_filter6 + op_ace_count_ingress_ipv6) > stm_max_ingress6) ||
            ((entry_count.cur_egress6 + op_ace_count_egress_ipv6) > stm_max_egress6))
        {
            return PM_E_CFG_NO_RESOURCE;
        }
    }

    return PM_E_NONE;
}


int32
acl_policy_operation_res_check(char* interface_name, uint32 direct_in, char* policy_map_name, char* class_map_name, char* acl_name, 
        uint32 ethertype_or_arp_flag, uint32 tcp_flags, glb_acl_policy_operation_type_t op_type)
{
     int ret = PM_E_NONE;
    switch(op_type)
    {
        case GLB_ACL_POLICY_OP_IF_PMAP:
              ret = acl_policy_operation_if_pmap_add_res_check(interface_name, direct_in, policy_map_name);
            break;
        case GLB_ACL_POLICY_OP_PMAP_CLASS:
              ret =  acl_policy_operation_pmap_class_add_res_check(policy_map_name, class_map_name);
            break;
        case GLB_ACL_POLICY_OP_CLASS_ACL:
             ret =  acl_policy_operation_class_acl_add_res_check(class_map_name, acl_name);
            break;
        case GLB_ACL_POLICY_OP_ACL_ACE:
             ret = acl_policy_operation_acl_ace_add_res_check(acl_name, ethertype_or_arp_flag, tcp_flags);
            break;
        default:
            break;
        
    }
    return ret;
}

