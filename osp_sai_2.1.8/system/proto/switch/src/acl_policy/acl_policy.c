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
#include "gen/tbl_clear_acl_policy_define.h"
#include "gen/tbl_clear_acl_policy.h"
#include "gen/tbl_tap_group_ingress_flow_define.h"
#include "gen/tbl_tap_group_ingress_flow.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "switch.h"
#include "acl.h"
#include "acl_policy.h"
#include "if_agg.h"
#include "acl_policy_res_check.h"

extern int32
acl_config_del_ace(char *acl_name, uint32 seq_no, char **argv, int32 argc, uint32 snmp_set);

extern int32
_acl_show_ace_config(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs);

extern glb_qos_color_t
qos_mgr_map_str2color(char* str);

extern int32
qos_mgr_policer_res_check(char *policer_name, bool direct_in, bool port_policer, uint32 *count);

extern int32
qos_mgr_policer_res_add(char *policer_name, bool direct_in, bool port_policer);

extern int32
qos_mgr_policer_res_del(char *policer_name, bool direct_in, bool port_policer);

int32
acl_entry_add_entry(tbl_ace_config_t *pst_ace, 
                    tbl_ace_config_t *pst_inner_ace, 
                    tbl_interface_t *pst_interface, 
                    uint32 direct_in, 
                    uint64 class_priority, 
                    uint64 group_priority,
                    char* class_map_name, 
                    char* policy_map_name, 
                    tbl_tap_group_ingress_flow_t *p_tap_grp_flow,
                    uint32 decap_isv6);
extern int32
acl_entry_del_entry(tbl_ace_config_t *pst_ace,
                    tbl_ace_config_t *pst_inner_ace,
                    tbl_interface_t *pst_interface, 
                    uint32 direct_in, 
                    uint64 class_priority, 
                    uint64 group_priority);

typedef enum 
{
    ACL_POLICY_POLICER_RES_CHECK,
    ACL_POLICY_POLICER_RES_ADD,
    ACL_POLICY_POLICER_RES_DEL,
    ACL_POLICY_POLICER_RES_MAX
} acl_policy_policer_res_op_t;

int32
acl_policy_attach_or_detach_interface_policer_res_op(char *policy_name, bool direct_in, acl_policy_policer_res_op_t op)
{
    tbl_class_map_action_config_master_t *p_action_config_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *action_config_listnode = NULL;
    ctclib_slistnode_t *action_config_next = NULL;
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;
    uint32 policer_res_cnt = 0;
    char policer_name[GLB_QOS_POLICER_PROFILE_NUM][POLICER_NAME_SIZE+1] = {{0}};
    uint32 policer_name_cnt = 0;
    bool flag = FALSE;
    uint32 i;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP_DEL(p_action_config_master->class_map_action_config_list, p_db_class_map_action_config, action_config_listnode, action_config_next)
    {
        if(0 == sal_strcmp(p_db_class_map_action_config->key.policy_name, policy_name))
        {
            if(GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
            {
                switch(op)
                {
                case ACL_POLICY_POLICER_RES_CHECK:
                    //ignore policer name has been checked
                    for (i = 0; i< policer_name_cnt; i++)
                    {
                        if (0 == sal_strcmp(p_db_class_map_action_config->policer, policer_name[i]))
                        {
                            flag = TRUE;
                            break;
                        }
                    }
                    if (!flag)
                    {
                        PM_E_RETURN(qos_mgr_policer_res_check(p_db_class_map_action_config->policer, direct_in, FALSE, &policer_res_cnt));
                        flag = FALSE;
                        sal_strcpy(policer_name[policer_name_cnt++], p_db_class_map_action_config->policer);
                    }
                    break;
                case ACL_POLICY_POLICER_RES_ADD:
                    PM_E_RETURN(qos_mgr_policer_res_add(p_db_class_map_action_config->policer, direct_in, FALSE));
                    break;
                case ACL_POLICY_POLICER_RES_DEL:
                    PM_E_RETURN(qos_mgr_policer_res_del(p_db_class_map_action_config->policer, direct_in, FALSE));
                    break;
                default:
                    break;
                }
            }
        }
    }
    return rc;
}

int32
acl_policy_class_action_policer_res_op(char *policy_name, char *class_name, char *policer_name, acl_policy_policer_res_op_t op)
{
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;
    tbl_class_map_action_config_key_t key;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    bool direct_in = TRUE;
    uint32 ingress_policer_res_cnt = 0;
    uint32 egress_policer_res_cnt = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&key, 0x0, sizeof(tbl_class_map_action_config_key_t));
    sal_strcpy(key.policy_name, policy_name);
    sal_strcpy(key.class_name, class_name);

    p_db_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config(&key);
    if (!p_db_class_map_action_config || 0 == sal_strlen(policer_name))
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if (0 != sal_strlen(p_db_if->igs_policy_map) && 0 == sal_strcmp(p_db_if->igs_policy_map, policy_name))
        {
            direct_in = TRUE;
            switch(op)
            {
            case ACL_POLICY_POLICER_RES_CHECK:
                PM_E_RETURN(qos_mgr_policer_res_check(policer_name, direct_in, FALSE, &ingress_policer_res_cnt));
                ingress_policer_res_cnt = 0;
                break;
            case ACL_POLICY_POLICER_RES_ADD:
                PM_E_RETURN(qos_mgr_policer_res_add(policer_name, direct_in, FALSE));
                break;
            case ACL_POLICY_POLICER_RES_DEL:
                PM_E_RETURN(qos_mgr_policer_res_del(policer_name, direct_in, FALSE));
                break;
            default:
                break;
            }
        }

        if (0 != sal_strlen(p_db_if->egs_policy_map) && 0 == sal_strcmp(p_db_if->egs_policy_map, policy_name))
        {
            direct_in = FALSE;
            switch(op)
            {
            case ACL_POLICY_POLICER_RES_CHECK:
                PM_E_RETURN(qos_mgr_policer_res_check(policer_name, direct_in, FALSE, &egress_policer_res_cnt));
                egress_policer_res_cnt = 0;
                break;
            case ACL_POLICY_POLICER_RES_ADD:
                PM_E_RETURN(qos_mgr_policer_res_add(policer_name, direct_in, FALSE));
                break;
            case ACL_POLICY_POLICER_RES_DEL:
                PM_E_RETURN(qos_mgr_policer_res_del(policer_name, direct_in, FALSE));
                break;
            default:
                break;
            }
        }
        
    }
    
    return PM_E_NONE;
}


int32
_acl_entry_attach_interface_by_policy(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    tbl_interface_t *pst_interface = pargs->argv[0];
    uint32 *direct_in              = pargs->argv[1];
    char* acl_name               = pargs->argv[2];
    uint32 *ret                    = pargs->argv[3];
    uint64 *class_priority      = pargs->argv[4];
    uint64 *group_priority        = pargs->argv[5];
    char* class_map_name      = pargs->argv[6];
    char* policy_map_name      = pargs->argv[7];

    if (*direct_in)
    {
        if (0 == sal_strcmp(p_ace_config->key.acl_name, acl_name))
        {
            *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, TRUE, *class_priority, *group_priority, class_map_name, policy_map_name, NULL, 0);
        }
    }
    else
    {
        if (0 == sal_strcmp(p_ace_config->key.acl_name, acl_name))
        {
            *ret = acl_entry_add_entry(p_ace_config, NULL, pst_interface, FALSE, *class_priority, *group_priority, class_map_name, policy_map_name, NULL, 0);
        }
    }

    return PM_E_NONE;
}

int32
acl_entry_attach_interface_by_policy(char *acl_name, tbl_interface_t *pst_interface, uint32 direct_in, uint64 class_priority, uint64 group_priority,
                                                             char* class_map_name, char* policy_map_name)
{
    tbl_iter_args_t args;
    int32 ret = 0;

    args.argv[0] = pst_interface;
    args.argv[1] = &direct_in;
    args.argv[2] = acl_name;
    args.argv[3] = &ret;
    args.argv[4] = &class_priority;
    args.argv[5] = &group_priority;
    args.argv[6] = class_map_name;
    args.argv[7] = policy_map_name;

    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_entry_attach_interface_by_policy, &args);
    return PM_E_NONE;
}


int32
_acl_entry_detach_interface_by_policy(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    tbl_interface_t *pst_interface = pargs->argv[0];
    uint32 *direct_in              = pargs->argv[1];
    char* acl_name               = pargs->argv[2];
    uint32 *ret                    = pargs->argv[3];
    uint64 *class_priority      = pargs->argv[4];
    uint64 *group_priority        = pargs->argv[5];

    if (*direct_in)
    {
        if (0 == sal_strcmp(p_ace_config->key.acl_name, acl_name))
        {
            *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, TRUE, *class_priority, *group_priority);
        }
    }
    else
    {
        if (0 == sal_strcmp(p_ace_config->key.acl_name, acl_name))
        {
            *ret = acl_entry_del_entry(p_ace_config, NULL, pst_interface, FALSE, *class_priority, *group_priority);
        }
    }
    return PM_E_NONE;
}

int32
acl_entry_detach_interface_by_policy(char *acl_name, tbl_interface_t *pst_interface, uint32 direct_in, uint64 class_priority, uint64 group_priority)
{
    tbl_iter_args_t args;
    int32 ret = 0;

    args.argv[0] = pst_interface;
    args.argv[1] = &direct_in;
    args.argv[2] = acl_name;
    args.argv[3] = &ret;
    args.argv[4] = &class_priority;
    args.argv[5] = &group_priority;
    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_acl_entry_detach_interface_by_policy, &args);
    return PM_E_NONE;
}

int32
acl_policy_operation_copp_pmap_add_action_check(char* policy_map_name)
{
    tbl_class_map_action_config_master_t *p_action_config_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *action_config_listnode = NULL;
    ctclib_slistnode_t *action_config_next = NULL;
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;


    CTCLIB_SLIST_LOOP_DEL(p_action_config_master->class_map_action_config_list, p_db_class_map_action_config, action_config_listnode, action_config_next)
    {
        if(0 == sal_strcmp(p_db_class_map_action_config->key.policy_name, policy_map_name))
        {
            /*modified by yejl for bug 49938 and bug 49973, 2018-11-13*/
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION))
            {
                return PM_E_CHECK_CMAP_MONITOR_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
            {
                return PM_E_CHECK_CMAP_POLICY_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
            {
                return PM_E_CHECK_CMAP_CTAG_COS_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
            {
                return PM_E_CHECK_CMAP_CTAG_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_DSCP))
            {
                return PM_E_CHECK_CMAP_DSCP_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
            {
                return PM_E_CHECK_CMAP_STAG_COS_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
            {
                return PM_E_CHECK_CMAP_STAG_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_COLOR))
            {
                return PM_E_CHECK_CMAP_TC_TO_COLOR_UNSUPPORT;
            }
            if ((GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE)) ||
                '\0' != p_db_class_map_action_config->red_unexist_agg[0])
            {
                return PM_E_CHECK_CMAP_REDIRECT_UNSUPPORT;
            }
            /*end*/
        }
    }

    return PM_E_NONE;
}


int32
acl_policy_operation_if_pmap_add_egress_action_check(char* policy_map_name)
{
    tbl_class_map_action_config_master_t *p_action_config_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *action_config_listnode = NULL;
    ctclib_slistnode_t *action_config_next = NULL;
    tbl_class_map_action_config_t *p_db_class_map_action_config = NULL;


    CTCLIB_SLIST_LOOP_DEL(p_action_config_master->class_map_action_config_list, p_db_class_map_action_config, action_config_listnode, action_config_next)
    {
        if (0 == sal_strcmp(p_db_class_map_action_config->key.policy_name, policy_map_name))
        {
            /*modified by yejl for bug 49938 and bug 49973, 2018-11-13*/
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
            {
                return PM_E_CHECK_CMAP_CTAG_COS_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
            {
                return PM_E_CHECK_CMAP_CTAG_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_DSCP))
            {
                return PM_E_CHECK_CMAP_DSCP_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
            {
                return PM_E_CHECK_CMAP_STAG_COS_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
            {
                return PM_E_CHECK_CMAP_STAG_UNSUPPORT;
            }
            if (GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_COLOR))
            {
                return PM_E_CHECK_CMAP_TC_TO_COLOR_UNSUPPORT;
            }
            if ((GLB_FLAG_ISSET(p_db_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE)) ||
                '\0' != p_db_class_map_action_config->red_unexist_agg[0])
            {
                return PM_E_CHECK_CMAP_REDIRECT_UNSUPPORT;
            }
            /*end*/
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_if_pmap_add_egress_ethertype_check(char* policy_map_name)
{
    tbl_class_in_policy_config_master_t *p_class_in_policy_master = tbl_class_in_policy_config_get_master();
    ctclib_slistnode_t *class_in_policy_listnode = NULL;
    ctclib_slistnode_t *class_in_policy_next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    tbl_acl_config_key_t acl_config_key;
    tbl_acl_config_t* p_db_acl_config = NULL;


    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, policy_map_name))
        {
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
                    sal_strcpy(acl_config_key.name, p_db_acl_in_class_config->key.acl_name);
                    p_db_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
                    if (NULL != p_db_acl_config)
                    {
                        if (p_db_acl_config->ether_ref)
                        {
                            return PM_E_CFG_PM_ERROR;
                        }
                    }
                }  
            }
        }
    }
    return PM_E_NONE;
}

int32
acl_policy_get_tcp_flags_config_count()
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
    
    tbl_acl_config_master_t * p_acl_master = tbl_acl_config_get_master();
    ctclib_slistnode_t *acl_config_listnode = NULL;
    ctclib_slistnode_t *acl_config_next = NULL;
    tbl_acl_config_t* p_db_acl_config = NULL;

    uint32 tcp_flags_count1 = 0;
    uint32 tcp_flags_count = 0;    

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if (0 != sal_strlen(p_db_if->igs_policy_map))
        {
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if (NULL != p_db_class_in_policy_config)
                {
                    if (0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->igs_policy_map))
                    {
                        CTCLIB_SLIST_LOOP_DEL(p_acl_master->acl_config_list, p_db_acl_config, acl_config_listnode, acl_config_next)
                        {
                            p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, p_db_acl_config->key.name);
                            if (NULL != p_db_acl_in_class_config)
                            {
                                tcp_flags_count1 = p_db_acl_config->tcp_flags_ref; 
                                tcp_flags_count = tcp_flags_count + tcp_flags_count1;
                            }
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
                        CTCLIB_SLIST_LOOP_DEL(p_acl_master->acl_config_list, p_db_acl_config, acl_config_listnode, acl_config_next)
                        {
                            p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, p_db_acl_config->key.name);
                            if (NULL != p_db_acl_in_class_config)
                            {
                                tcp_flags_count1 = p_db_acl_config->tcp_flags_ref; 
                                tcp_flags_count = tcp_flags_count + tcp_flags_count1;
                            }
                        }
                    }
                }
            }
        }
    }
    return tcp_flags_count;
}


int32
acl_policy_action_update_action_check(char* policy_map_name)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 == sal_strcmp(p_db_if->egs_policy_map, policy_map_name))
        {
            return PM_E_CHECK_CMAP_POLICY_APPLY_ON_EGRESS_PORT;
        }
    }
    return PM_E_NONE;
}


int32
acl_copp_policy_action_update_action_check(char* policy_map_name)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 == sal_strcmp(p_db_if->igs_policy_map, policy_map_name) &&
           0 == sal_strcmp(p_db_if->key.name, GLB_IFNAME_CPU_PREFIX))
        {
            return PM_E_CHECK_CMAP_POLICY_APPLY_ON_COPP;
        }
    }
    return PM_E_NONE;
}

int32
acl_policy_operation_if_pmap_add(char* interface_name, uint32 direct_in, char* policy_map_name)
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

    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_NOT_EXIST;
    }
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config_by_name(policy_map_name);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }

    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, policy_map_name))
        {
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    acl_entry_attach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, pst_interface, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, p_db_acl_in_class_config->key.class_name, policy_map_name);
                }
            }

        }
  
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_if_pmap_del(char* interface_name, uint32 direct_in, char* policy_map_name)
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

    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_NOT_EXIST;
    }
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config_by_name(policy_map_name);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }


    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, policy_map_name))
        {
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    acl_entry_detach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, pst_interface, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority);
                }
            }

        }
  
    }
    return PM_E_NONE;
}


int32
acl_policy_operation_pmap_class_add(char*policy_map_name, char*class_map_name)
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

    uint32 direct_in = 0;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 == sal_strcmp(p_db_if->igs_policy_map, policy_map_name))
        {
            direct_in = 1;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            if (NULL == p_db_class_in_policy_config)
            {
                return PM_E_NONE;
            }
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    acl_entry_attach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, class_map_name, policy_map_name);
                }
            }
        }

        if(0 == sal_strcmp(p_db_if->egs_policy_map, policy_map_name))
        {
            direct_in = 0;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            if (NULL == p_db_class_in_policy_config)
            {
                return PM_E_NONE;
            }
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    acl_entry_attach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, class_map_name, policy_map_name);
                }
            }
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_pmap_class_del(char*policy_map_name, char*class_map_name)
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

    uint32 direct_in = 0;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 == sal_strcmp(p_db_if->igs_policy_map, policy_map_name))
        {
            direct_in = 1;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            if(NULL == p_db_class_in_policy_config)
            {
                return PM_E_NONE;
            }
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    acl_entry_detach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority);
                }
            }
        }

        if(0 == sal_strcmp(p_db_if->egs_policy_map, policy_map_name))
        {
            direct_in = 0;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            if(NULL == p_db_class_in_policy_config)
            {
                return PM_E_NONE;
            }
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    acl_entry_detach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority);
                }
            }
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_class_acl_add(char*class_map_name, char*acl_name)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    uint32 direct_in = 0;


    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 != sal_strlen(p_db_if->igs_policy_map))
        {
            direct_in = 1;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->igs_policy_map, class_map_name);
            if(NULL != p_db_class_in_policy_config)
            {
                p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(class_map_name, acl_name);
                if (NULL == p_db_acl_in_class_config)
                {
                    return PM_E_NONE;
                }
                acl_entry_attach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, class_map_name, p_db_if->igs_policy_map);
            }
        }

        if(0 != sal_strlen(p_db_if->egs_policy_map))
        {
            direct_in = 0;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->egs_policy_map, class_map_name);
            if(NULL != p_db_class_in_policy_config)
            {
                p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(class_map_name, acl_name);
                if (NULL == p_db_acl_in_class_config)
                {
                    return PM_E_NONE;
                }
                acl_entry_attach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, class_map_name, p_db_if->egs_policy_map);
            }
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_class_acl_del(char*class_map_name, char*acl_name)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;
    uint32 direct_in = 0;


    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 != sal_strlen(p_db_if->igs_policy_map))
        {
            direct_in = 1;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->igs_policy_map, class_map_name);
            if(NULL != p_db_class_in_policy_config)
            {
                p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(class_map_name, acl_name);
                if (NULL == p_db_acl_in_class_config)
                {
                    return PM_E_NONE;
                }
                acl_entry_detach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority);
            }
        }

        if(0 != sal_strlen(p_db_if->egs_policy_map))
        {
            direct_in = 0;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->egs_policy_map, class_map_name);
            if(NULL != p_db_class_in_policy_config)
            {
                p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(class_map_name, acl_name);
                if (NULL == p_db_acl_in_class_config)
                {
                    return PM_E_NONE;
                }
                acl_entry_detach_interface_by_policy(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority);
            }
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_acl_ace_add(char*acl_name, uint32 seq_no)
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
    uint32 direct_in = 0;
    tbl_ace_config_t *p_ace_config = NULL;
    tbl_iter_args_t args;
    int32 ret = 0;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 != sal_strlen(p_db_if->igs_policy_map))
        {
            direct_in = 1;
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if(NULL != p_db_class_in_policy_config)
                {
                    if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->igs_policy_map))
                    {
                        p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, acl_name);
                        if(NULL != p_db_acl_in_class_config)
                        {
                            p_ace_config = tbl_ace_config_get_ace_config_by_name(acl_name, seq_no);
                            if (NULL == p_ace_config)
                            {
                                return PM_E_NONE;
                            }
                            sal_memset(&args, 0, sizeof(tbl_iter_args_t));
                            args.argv[0] = p_db_if;
                            args.argv[1] = &direct_in;
                            args.argv[2] = acl_name;
                            args.argv[3] = &ret;
                            args.argv[4] = &p_db_class_in_policy_config->class_priority;
                            args.argv[5] = &p_db_acl_in_class_config->acl_priority;
                            args.argv[6] = p_db_class_in_policy_config->key.class_name;
                            args.argv[7] = p_db_if->igs_policy_map;
                            _acl_entry_attach_interface_by_policy(p_ace_config, &args);
                        }
                    }

                }
            }
        }

        if(0 != sal_strlen(p_db_if->egs_policy_map))
        {
            direct_in = 0;
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if(NULL != p_db_class_in_policy_config)
                {
                    if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->egs_policy_map))
                    {
                        p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, acl_name);
                        if(NULL != p_db_acl_in_class_config)
                        {
                            p_ace_config = tbl_ace_config_get_ace_config_by_name(acl_name, seq_no);
                            if (NULL == p_ace_config)
                            {
                                return PM_E_NONE;
                            }
                            sal_memset(&args, 0, sizeof(tbl_iter_args_t));
                            args.argv[0] = p_db_if;
                            args.argv[1] = &direct_in;
                            args.argv[2] = acl_name;
                            args.argv[3] = &ret;
                            args.argv[4] = &p_db_class_in_policy_config->class_priority;
                            args.argv[5] = &p_db_acl_in_class_config->acl_priority;
                            args.argv[6] = p_db_class_in_policy_config->key.class_name;
                            args.argv[7] = p_db_if->egs_policy_map;
                            _acl_entry_attach_interface_by_policy(p_ace_config, &args);
                        }
                    }
                }
            }
        }
    }

    return PM_E_NONE;
}


int32
acl_policy_operation_acl_ace_del(char*acl_name, uint32 seq_no)
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
    uint32 direct_in = 0;
    tbl_ace_config_t *p_ace_config = NULL;
    tbl_iter_args_t args;
    int32 ret = 0;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 != sal_strlen(p_db_if->igs_policy_map))
        {
            direct_in = 1;
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if(NULL != p_db_class_in_policy_config)
                {
                    if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->igs_policy_map))
                    {
                        p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, acl_name);
                        if(NULL != p_db_acl_in_class_config)
                        {
                            p_ace_config = tbl_ace_config_get_ace_config_by_name(acl_name, seq_no);
                            if (NULL == p_ace_config)
                            {
                                return PM_E_NONE;
                            }
                            sal_memset(&args, 0, sizeof(tbl_iter_args_t));
                            args.argv[0] = p_db_if;
                            args.argv[1] = &direct_in;
                            args.argv[2] = acl_name;
                            args.argv[3] = &ret;
                            args.argv[4] = &p_db_class_in_policy_config->class_priority;
                            args.argv[5] = &p_db_acl_in_class_config->acl_priority;
                            _acl_entry_detach_interface_by_policy(p_ace_config, &args);
                        }
                    }

                }
            }
        }

        if(0 != sal_strlen(p_db_if->egs_policy_map))
        {
            direct_in = 0;
            CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
            {
                if(NULL != p_db_class_in_policy_config)
                {
                    if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, p_db_if->egs_policy_map))
                    {
                        p_db_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_name(p_db_class_in_policy_config->key.class_name, acl_name);
                        if(NULL != p_db_acl_in_class_config)
                        {
                            p_ace_config = tbl_ace_config_get_ace_config_by_name(acl_name, seq_no);
                            if (NULL == p_ace_config)
                            {
                                return PM_E_NONE;
                            }
                            sal_memset(&args, 0, sizeof(tbl_iter_args_t));
                            args.argv[0] = p_db_if;
                            args.argv[1] = &direct_in;
                            args.argv[2] = acl_name;
                            args.argv[3] = &ret;
                            args.argv[4] = &p_db_class_in_policy_config->class_priority;
                            args.argv[5] = &p_db_acl_in_class_config->acl_priority;
                            _acl_entry_detach_interface_by_policy(p_ace_config, &args);
                        }
                    }

                }
            }
        }
    }

    return PM_E_NONE;
}


int32
acl_policy_operation_if_pmap(char* interface_name, uint32 direct_in, char* policy_map_name, glb_acl_policy_op_action_type_t op_action)
{
    switch(op_action)
    {
        case GLB_ACL_POLICY_OP_ACTION_ADD:
            acl_policy_operation_if_pmap_add(interface_name, direct_in, policy_map_name);
            break;
        case GLB_ACL_POLICY_OP_ACTION_DEL:
            acl_policy_operation_if_pmap_del(interface_name, direct_in, policy_map_name);
            break;
        case GLB_ACL_POLICY_OP_ACTION_UPDATE:
            break;
        default:
            break;
    }

    return PM_E_NONE;
}


int32
acl_policy_operation_pmap_class(char* policy_map_name, char* class_map_name, glb_acl_policy_op_action_type_t op_action)
{
    switch(op_action)
    {
        case GLB_ACL_POLICY_OP_ACTION_ADD:
            acl_policy_operation_pmap_class_add(policy_map_name, class_map_name);
            break;
        case GLB_ACL_POLICY_OP_ACTION_DEL:
            acl_policy_operation_pmap_class_del(policy_map_name, class_map_name);
            break;
        case GLB_ACL_POLICY_OP_ACTION_UPDATE:
            break;
        default:
            break;
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_class_acl(char* class_map_name, char* acl_name, glb_acl_policy_op_action_type_t op_action)
{
    switch(op_action)
    {
        case GLB_ACL_POLICY_OP_ACTION_ADD:
            acl_policy_operation_class_acl_add(class_map_name, acl_name);
            break;
        case GLB_ACL_POLICY_OP_ACTION_DEL:
            acl_policy_operation_class_acl_del(class_map_name, acl_name);
            break;
        case GLB_ACL_POLICY_OP_ACTION_UPDATE:
            break;
        default:
            break;
    }

    return PM_E_NONE;
}

int32
acl_policy_operation_acl_ace(char* acl_name, uint32 seq_no, glb_acl_policy_op_action_type_t op_action)
{
    switch(op_action)
    {
        case GLB_ACL_POLICY_OP_ACTION_ADD:
            acl_policy_operation_acl_ace_add(acl_name, seq_no);
            break;
        case GLB_ACL_POLICY_OP_ACTION_DEL:
            acl_policy_operation_acl_ace_del(acl_name, seq_no);
            break;
        case GLB_ACL_POLICY_OP_ACTION_UPDATE:
            break;
        default:
            break;
    }

    return PM_E_NONE;
}

int32
acl_policy_operation(char* interface_name, uint32 direct_in, char* policy_map_name, char* class_map_name, char* acl_name, uint32 seq_no,
                    glb_acl_policy_operation_type_t op_type, glb_acl_policy_op_action_type_t op_action)
{
    switch(op_type)
    {
        case GLB_ACL_POLICY_OP_IF_PMAP:
                acl_policy_operation_if_pmap(interface_name, direct_in, policy_map_name, op_action);
            break;
        case GLB_ACL_POLICY_OP_PMAP_CLASS:
                acl_policy_operation_pmap_class(policy_map_name, class_map_name, op_action);
            break;
        case GLB_ACL_POLICY_OP_CLASS_ACL:
                acl_policy_operation_class_acl(class_map_name, acl_name, op_action);
            break;
        case GLB_ACL_POLICY_OP_ACL_ACE:
                acl_policy_operation_acl_ace(acl_name, seq_no, op_action);
            break;
        default:
            break;
        
    }
    return PM_E_NONE;
}

int32
_ace_attach_acl_rank_update_priority(tbl_ace_config_t *p_ace_config, tbl_iter_args_t *pargs)
{
    tbl_interface_t *pst_interface = pargs->argv[0];
    uint32 *direct_in              = pargs->argv[1];
    char* acl_name               = pargs->argv[2];
    uint32 *ret                    = pargs->argv[3];
    uint64 *class_priority      = pargs->argv[4];
    uint64 *group_priority        = pargs->argv[5];
    uint64 *new_class_priority  = pargs->argv[6];
    uint64 *new_group_priority  = pargs->argv[7];

    uint64 acl_id = 0;
    uint64 type = 0;
    uint32 new_entry_priority = 0;
    tbl_acl_entry_key_t acl_entry_key;
    tbl_acl_entry_t* pst_acl_entry = NULL;

    if (0 == sal_strcmp(p_ace_config->key.acl_name, acl_name))
    {
        if (p_ace_config->is_ipv6 || p_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
        {
            type = GLB_ACL_CONFIG_TYPE_IPV6;
        }
        else if (p_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
        {
            type = GLB_ACL_CONFIG_TYPE_IPV6;
        }
        else if (p_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
        {
            type = GLB_ACL_CONFIG_TYPE_NORMAL;
        }
        acl_id = MAKE_ACL_ID(p_ace_config->key.seq_no, *direct_in, pst_interface->ifindex, *class_priority, *group_priority, type);
        sal_memset(&acl_entry_key, 0x0, sizeof(tbl_acl_entry_key_t));
        acl_entry_key.aclid = acl_id;
        pst_acl_entry = tbl_acl_entry_get_acl_entry(&acl_entry_key);
        if(NULL == pst_acl_entry)
        {
            return PM_E_NOT_EXIST;
        }
        new_entry_priority =  MAKE_ACL_PRIORITY(p_ace_config->key.seq_no, *new_class_priority, *new_group_priority);
        pst_acl_entry->entry_priority = new_entry_priority;
        *ret = tbl_acl_entry_set_acl_entry_field(pst_acl_entry, TBL_ACL_ENTRY_FLD_ENTRY_PRIORITY);
    }

    return *ret;
}



int32
ace_attach_acl_rank_update_priority(char *acl_name, tbl_interface_t *pst_interface, uint32 direct_in, uint64 class_priority, uint64 group_priority, 
                                                         uint64 new_class_priority, uint64 new_group_priority)
{
    tbl_iter_args_t args;
    int32 ret = 0;

    args.argv[0] = pst_interface;
    args.argv[1] = &direct_in;
    args.argv[2] = acl_name;
    args.argv[3] = &ret;
    args.argv[4] = &class_priority;
    args.argv[5] = &group_priority;
    args.argv[6] = &new_class_priority;
    args.argv[7] = &new_group_priority;

    tbl_ace_config_iterate((TBL_ITER_CB_FUNC)_ace_attach_acl_rank_update_priority, &args);
    return PM_E_NONE;
}

int32 class_attach_policy_rank_update_priority(tbl_class_in_policy_config_t* pst_class_in_policy_config, uint64 new_class_priority)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    uint32 direct_in = 0;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 == sal_strcmp(p_db_if->igs_policy_map, pst_class_in_policy_config->key.policy_name))
        {
            direct_in = 1;
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, pst_class_in_policy_config->key.class_name))
                {
                    ace_attach_acl_rank_update_priority(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  pst_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, new_class_priority, p_db_acl_in_class_config->acl_priority);
                }
            }
        }

        if(0 == sal_strcmp(p_db_if->egs_policy_map, pst_class_in_policy_config->key.policy_name))
        {
            direct_in = 0;
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, pst_class_in_policy_config->key.class_name))
                {
                    ace_attach_acl_rank_update_priority(p_db_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  pst_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, new_class_priority, p_db_acl_in_class_config->acl_priority);
                }
            }
        }
    }
    return PM_E_NONE;
}



int32
class_attach_policy_rank(char *class_map_name)
{
    uint32 current_max_priority = 1;
    uint64 priority_index = 0;
    tbl_class_in_policy_config_t* pst_class_in_policy_config = NULL;

    for(priority_index = 1; priority_index <= GLB_CLASS_IN_POLCIY_MAX; priority_index ++)
    {
        pst_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_class_priority(priority_index);
        if(NULL == pst_class_in_policy_config)
        {
            continue;
        }
        if(pst_class_in_policy_config->class_priority != current_max_priority)
        {
            /*update priority*/
            class_attach_policy_rank_update_priority(pst_class_in_policy_config, current_max_priority);
            pst_class_in_policy_config->class_priority = current_max_priority;
            tbl_class_in_policy_config_set_class_in_policy_config_field(pst_class_in_policy_config, TBL_CLASS_IN_POLICY_CONFIG_FLD_CLASS_PRIORITY);    
        }
        current_max_priority ++;    
    }

    return PM_E_NONE;
}

int32
class_attach_policy(char *policy_map_name, char *class_map_name, cfg_cmd_para_t *para)
{
    tbl_policy_map_config_t *pst_policy_map_config = NULL;
    tbl_policy_map_config_key_t policy_map_config_key;
    tbl_class_map_config_t *pst_class_map_config = NULL;
    tbl_class_map_config_key_t class_map_config_key;
    tbl_class_in_policy_config_t *pst_class_in_policy_config = NULL;
    tbl_class_in_policy_config_t class_in_policy_config;
    int32 ret  = PM_E_NONE;

    /*get policy map config*/
    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_NOT_PERMIT;
    }
    sal_memset(&policy_map_config_key, 0, sizeof(policy_map_config_key));
    sal_strcpy(policy_map_config_key.name, policy_map_name);
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config(&policy_map_config_key);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }

    /*get class map config*/
    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&class_map_config_key, 0, sizeof(class_map_config_key));
    sal_strcpy(class_map_config_key.name, class_map_name);
    pst_class_map_config = tbl_class_map_config_get_class_map_config(&class_map_config_key);
    if (NULL == pst_class_map_config)
    {
        return PM_E_CMAP_NOT_EXIST;
    }

    /*add or update class in policy config table*/
    sal_memset(&class_in_policy_config, 0, sizeof(class_in_policy_config));
    sal_strcpy(class_in_policy_config.key.policy_name, policy_map_name);
    sal_strcpy(class_in_policy_config.key.class_name, class_map_name);
    pst_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config(&class_in_policy_config.key);
    if (NULL == pst_class_in_policy_config)
    {
        if (0 == pst_policy_map_config->class_ref)
        {
            pst_policy_map_config->max_class_priority = 0;
        }
        if (GLB_CLASS_IN_POLCIY_MAX <= pst_policy_map_config->class_ref)
        {
            return PM_E_RESOURCE_FULL;
        }
        if((GLB_CLASS_IN_POLCIY_MAX == pst_policy_map_config->max_class_priority) && (GLB_CLASS_IN_POLCIY_MAX != pst_policy_map_config->class_ref))
        {
            class_attach_policy_rank(policy_map_name);
            pst_policy_map_config->max_class_priority = pst_policy_map_config->class_ref;
        }
        pst_policy_map_config->max_class_priority ++;
        tbl_policy_map_config_set_policy_map_config_field(pst_policy_map_config, TBL_POLICY_MAP_CONFIG_FLD_MAX_CLASS_PRIORITY);
        
        pst_policy_map_config->class_ref ++;
        tbl_policy_map_config_set_policy_map_config_field(pst_policy_map_config, TBL_POLICY_MAP_CONFIG_FLD_CLASS_REF);

        tbl_class_in_policy_config_add_class_in_policy_config(&class_in_policy_config);
        pst_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config(&class_in_policy_config.key);
        if (NULL == pst_class_in_policy_config)
        {
            return PM_E_NO_MEMORY;
        }

        pst_class_in_policy_config->class_priority = pst_policy_map_config->max_class_priority;
        tbl_class_in_policy_config_set_class_in_policy_config_field(pst_class_in_policy_config, TBL_CLASS_IN_POLICY_CONFIG_FLD_CLASS_PRIORITY);

        ret = acl_policy_operation_res_check(NULL, 0, policy_map_name, class_map_name, NULL, 0, 0, GLB_ACL_POLICY_OP_PMAP_CLASS);
        if (ret != PM_E_NONE)
        {
            pst_policy_map_config->class_ref--;
            tbl_policy_map_config_set_policy_map_config_field(pst_policy_map_config, TBL_POLICY_MAP_CONFIG_FLD_CLASS_REF);

            tbl_class_in_policy_config_del_class_in_policy_config(&class_in_policy_config.key);
            return ret;
        }
        acl_policy_operation(NULL, 0, policy_map_name, class_map_name, NULL, 0, GLB_ACL_POLICY_OP_PMAP_CLASS, GLB_ACL_POLICY_OP_ACTION_ADD);

    }

    return PM_E_NONE;
}


int32
class_detach_policy(char *policy_map_name, char *class_map_name)
{
    tbl_policy_map_config_t *pst_policy_map_config = NULL;
    tbl_policy_map_config_key_t policy_map_config_key;
    tbl_class_map_config_t *pst_class_map_config = NULL;
    tbl_class_map_config_key_t class_map_config_key;
    tbl_class_in_policy_config_t * pst_class_in_policy_config = NULL;
    tbl_class_in_policy_config_t  class_in_policy_config;
    tbl_class_map_action_config_key_t class_map_action_config_key;
    tbl_qos_policer_profile_t *pst_qos_policer_profile = NULL;
    tbl_class_map_action_config_t *pst_class_map_action_config = NULL;
    int32 rc = PM_E_NONE;

    /*get policy map config*/
    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_NOT_PERMIT;
    }
    sal_memset(&policy_map_config_key, 0, sizeof(policy_map_config_key));
    sal_strcpy(policy_map_config_key.name, policy_map_name);
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config(&policy_map_config_key);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }

    /*get class map config*/
    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&class_map_config_key, 0, sizeof(class_map_config_key));
    sal_strcpy(class_map_config_key.name, class_map_name);
    pst_class_map_config = tbl_class_map_config_get_class_map_config(&class_map_config_key);
    if (NULL == pst_class_map_config)
    {
        return PM_E_CMAP_NOT_EXIST;
    }
    
    /*get class in policy config table*/
    sal_memset(&class_in_policy_config, 0, sizeof(class_in_policy_config));
    sal_strcpy(class_in_policy_config.key.policy_name, policy_map_name);
    sal_strcpy(class_in_policy_config.key.class_name, class_map_name);
    pst_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config(&class_in_policy_config.key);
    if (NULL == pst_class_in_policy_config)
    {
        return PM_E_CLASS_IN_POLICY_NOT_EXIST;
    }

    acl_policy_operation(NULL, 0, policy_map_name, class_map_name, NULL, 0, GLB_ACL_POLICY_OP_PMAP_CLASS, GLB_ACL_POLICY_OP_ACTION_DEL);

    /*added by hansf for bug 40670*/
    sal_memset(&class_map_action_config_key, 0, sizeof(class_map_action_config_key));
    sal_strcpy(class_map_action_config_key.policy_name, policy_map_name);
    sal_strcpy(class_map_action_config_key.class_name, class_map_name);
    pst_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config(&class_map_action_config_key);
    if (pst_class_map_action_config)
    {
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
        {
            pst_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(pst_class_map_action_config->policer);
            if(pst_qos_policer_profile)
            {
                pst_qos_policer_profile->ref_cnt --;
                tbl_qos_policer_profile_set_qos_policer_profile_field(pst_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT);
                /*added by hansf 2018/10/09 for policer resource del*/
                PM_E_RETURN(acl_policy_class_action_policer_res_op(policy_map_name, class_map_name, pst_class_map_action_config->policer, ACL_POLICY_POLICER_RES_DEL));
            }
        }
    }
    /*ended by hansf*/            
    
    pst_policy_map_config->class_ref--;
    tbl_policy_map_config_set_policy_map_config_field(pst_policy_map_config, TBL_POLICY_MAP_CONFIG_FLD_CLASS_REF);

    tbl_class_map_action_config_del_class_map_action_config(&class_map_action_config_key);

    tbl_class_in_policy_config_del_class_in_policy_config(&class_in_policy_config.key);
   
    return PM_E_NONE;
}

int32 acl_attach_class_rank_update_priority(tbl_acl_in_class_config_t* pst_acl_in_class_config, uint64 new_acl_group_priority)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    uint32 direct_in = 0;

    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 != sal_strlen(p_db_if->igs_policy_map))
        {
            direct_in = 1;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->igs_policy_map, pst_acl_in_class_config->key.class_name);
            if(NULL != p_db_class_in_policy_config)
            {
                ace_attach_acl_rank_update_priority(pst_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, pst_acl_in_class_config->acl_priority, p_db_class_in_policy_config->class_priority, new_acl_group_priority);
            }
        }

        if(0 != sal_strlen(p_db_if->egs_policy_map))
        {
            direct_in = 0;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_if->igs_policy_map, pst_acl_in_class_config->key.class_name);
            if(NULL != p_db_class_in_policy_config)
            {
                ace_attach_acl_rank_update_priority(pst_acl_in_class_config->key.acl_name, p_db_if, direct_in, 
                                                  p_db_class_in_policy_config->class_priority, pst_acl_in_class_config->acl_priority, p_db_class_in_policy_config->class_priority, new_acl_group_priority);
            }
        }
    }    

    return PM_E_NONE;
}


int32
acl_attach_class_rank(char *class_map_name)
{
    uint32 current_max_priority = 1;
    uint64 priority_index = 0;
    tbl_acl_in_class_config_t* pst_acl_in_class_config = NULL;

    for(priority_index = 1; priority_index <= GLB_ACL_IN_CLASS_MAX; priority_index ++)
    {
        pst_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config_by_acl_priority(priority_index);
        if(NULL == pst_acl_in_class_config)
        {
            continue;
        }
        if(pst_acl_in_class_config->acl_priority != current_max_priority)
        {
            /*update priority*/
            acl_attach_class_rank_update_priority(pst_acl_in_class_config, current_max_priority);
            pst_acl_in_class_config->acl_priority = current_max_priority;
            tbl_acl_in_class_config_set_acl_in_class_config_field(pst_acl_in_class_config, TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY);         
        }
        current_max_priority ++;  

    }

    return PM_E_NONE;
}


int32
acl_attach_class(char *class_map_name, char *acl_group_name, cfg_cmd_para_t *para)
{
    tbl_class_map_config_t *pst_class_map_config = NULL;
    tbl_class_map_config_key_t class_map_config_key;
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    tbl_acl_in_class_config_t *pst_acl_in_class_config = NULL;
    tbl_acl_in_class_config_t  acl_in_class_config;
    int32 ret = PM_E_NONE;

    /*get class map config*/
    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_NOT_PERMIT;
    }
    sal_memset(&class_map_config_key, 0, sizeof(class_map_config_key));
    sal_strcpy(class_map_config_key.name, class_map_name);
    pst_class_map_config = tbl_class_map_config_get_class_map_config(&class_map_config_key);
    if (NULL == pst_class_map_config)
    {
        return PM_E_CMAP_NOT_EXIST;
    }

    /*get acl group config*/
    if (check_name_character_and_len(acl_group_name, ACL_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
    sal_strcpy(acl_config_key.name, acl_group_name);
    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
        return PM_E_ACL_NOT_EXIST;
    }
    
    /*add or get acl in class config table*/
    sal_memset(&acl_in_class_config, 0, sizeof(acl_in_class_config));
    sal_strcpy(acl_in_class_config.key.class_name, class_map_name);
    sal_strcpy(acl_in_class_config.key.acl_name, acl_group_name);
    pst_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config(&acl_in_class_config.key);
    if (NULL == pst_acl_in_class_config)
    {
        if (0 == pst_class_map_config->acl_ref)
        {
            pst_class_map_config->max_acl_priority = 0;
        }
        if (GLB_ACL_IN_CLASS_MAX <= pst_class_map_config->acl_ref)
        {
            /*modifiled by yejl for bug 49926, 2018-11-12*/
            return PM_E_RESOURCE_FULL;
        }
        if((GLB_ACL_IN_CLASS_MAX == pst_class_map_config->max_acl_priority) && (GLB_ACL_IN_CLASS_MAX != pst_class_map_config->acl_ref))
        {
            acl_attach_class_rank(class_map_name);
            pst_class_map_config->max_acl_priority = pst_class_map_config->acl_ref;
        }
        pst_class_map_config->max_acl_priority ++;
        tbl_class_map_config_set_class_map_config_field(pst_class_map_config, TBL_CLASS_MAP_CONFIG_FLD_MAX_ACL_PRIORITY);
        
        pst_class_map_config->acl_ref ++;
        tbl_class_map_config_set_class_map_config_field(pst_class_map_config, TBL_CLASS_MAP_CONFIG_FLD_ACL_REF);

        tbl_acl_in_class_config_add_acl_in_class_config(&acl_in_class_config);

        pst_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config(&acl_in_class_config.key);
        if (NULL == pst_acl_in_class_config)
        {
            return PM_E_NO_MEMORY;
        }

        pst_acl_in_class_config->acl_priority = pst_class_map_config->max_acl_priority;
        tbl_acl_in_class_config_set_acl_in_class_config_field(pst_acl_in_class_config, TBL_ACL_IN_CLASS_CONFIG_FLD_ACL_PRIORITY);

        ret = acl_policy_operation_res_check(NULL, 0, NULL, class_map_name, acl_group_name, 0, 0, GLB_ACL_POLICY_OP_CLASS_ACL);
        if(ret != PM_E_NONE)
        {
            pst_class_map_config->acl_ref--;
            tbl_class_map_config_set_class_map_config_field(pst_class_map_config, TBL_CLASS_MAP_CONFIG_FLD_POLICY_MAP_REF);
           
            tbl_acl_in_class_config_del_acl_in_class_config(&acl_in_class_config.key);

            return ret;
        }
        acl_policy_operation(NULL, 0, NULL, class_map_name, acl_group_name, 0, GLB_ACL_POLICY_OP_CLASS_ACL, GLB_ACL_POLICY_OP_ACTION_ADD);

    }
    else
    {
        return PM_E_EXIST;
    }

    return PM_E_NONE;
}

int32
acl_detach_class(char *class_map_name, char *acl_group_name)
{
    tbl_class_map_config_t *pst_class_map_config = NULL;
    tbl_class_map_config_key_t class_map_config_key;
    tbl_acl_config_t *pst_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;
    tbl_acl_in_class_config_t *pst_acl_in_class_config = NULL;
    tbl_acl_in_class_config_key_t  acl_in_class_config_key;

    /*get class map config*/
    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_NOT_PERMIT;
    }
    sal_memset(&class_map_config_key, 0, sizeof(class_map_config_key));
    sal_strcpy(class_map_config_key.name, class_map_name);
    pst_class_map_config = tbl_class_map_config_get_class_map_config(&class_map_config_key);
    if (NULL == pst_class_map_config)
    {
        return PM_E_CMAP_NOT_EXIST;
    }

    /*get acl group config*/
    if (check_name_character_and_len(acl_group_name, ACL_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&acl_config_key, 0, sizeof(acl_config_key));
    sal_strcpy(acl_config_key.name, acl_group_name);
    pst_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == pst_acl_config)
    {
       return PM_E_ACL_NOT_EXIST;
    }
    
    /*get acl in class config table*/
    sal_memset(&acl_in_class_config_key, 0, sizeof(acl_in_class_config_key));
    sal_strcpy(acl_in_class_config_key.class_name, class_map_name);
    sal_strcpy(acl_in_class_config_key.acl_name, acl_group_name);
    pst_acl_in_class_config = tbl_acl_in_class_config_get_acl_in_class_config(&acl_in_class_config_key);
    if (NULL == pst_acl_in_class_config)
    {
       return PM_E_ACL_IN_CLASS_NOT_EXIST;
    }

    acl_policy_operation(NULL, 0, NULL, class_map_name, acl_group_name, 0, GLB_ACL_POLICY_OP_CLASS_ACL, GLB_ACL_POLICY_OP_ACTION_DEL);
    
    pst_class_map_config->acl_ref--;
    tbl_class_map_config_set_class_map_config_field(pst_class_map_config, TBL_CLASS_MAP_CONFIG_FLD_POLICY_MAP_REF);
   
    tbl_acl_in_class_config_del_acl_in_class_config(&acl_in_class_config_key);

    return PM_E_NONE;
}


int32
policy_map_attach_interface(char *policy_map_name, char *interface_name, bool direct_in)
{
    tbl_interface_t *pst_interface   = NULL;
    tbl_policy_map_config_t *pst_policy_map_config = NULL;
    tbl_policy_map_config_key_t policy_map_config_key;
    tbl_copp_cfg_t *p_copp_db_glb = tbl_copp_cfg_get_copp_cfg();
    int32 ret = PM_E_NONE;
    
    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_NOT_EXIST;
    }

    sal_memset(&policy_map_config_key, 0, sizeof(tbl_policy_map_config_key_t));
    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_strcpy(policy_map_config_key.name, policy_map_name);
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config(&policy_map_config_key);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }

    ret = acl_policy_operation_res_check(interface_name, direct_in, policy_map_name, NULL, NULL, 0, 0, GLB_ACL_POLICY_OP_IF_PMAP);
    if (ret == PM_E_NONE)
    {
        /* added by hansf for 2018/10/09 for policer resource check */
        ret = acl_policy_attach_or_detach_interface_policer_res_op(policy_map_name, direct_in, ACL_POLICY_POLICER_RES_CHECK);
        if (ret)
        {
            return ret;
        }
    }
    else
    {
        return ret;
    }

    if (!direct_in)
    {
        ret = acl_policy_operation_if_pmap_add_egress_action_check(policy_map_name);
        if(ret != PM_E_NONE)
        {
            return ret;
        }

        /*modified by yejl for bug 49782, 2018-11-01*/
        if (GLB_CHIP_GREATEBELT == tbl_chip_get_type())
        {
            ret = acl_policy_operation_if_pmap_add_egress_ethertype_check(policy_map_name);
            if (ret != PM_E_NONE)
            {
                return ret;    
            }
        }
        /*end*/
    }
    acl_policy_operation(interface_name, direct_in, policy_map_name, NULL, NULL, 0, GLB_ACL_POLICY_OP_IF_PMAP, GLB_ACL_POLICY_OP_ACTION_ADD);
    if (direct_in)
    {
        sal_strcpy(pst_interface->igs_policy_map, policy_map_name);
        tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_POLICY_MAP);
    }
    else
    {
        sal_strcpy(pst_interface->egs_policy_map, policy_map_name);
        tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_POLICY_MAP);
    }

    /* added by hansf for 2018/10/09 for policer resource add*/
    ret = acl_policy_attach_or_detach_interface_policer_res_op(policy_map_name, direct_in, ACL_POLICY_POLICER_RES_ADD);
    if (ret != PM_E_NONE)
    {
        return ret;
    }

    pst_policy_map_config->intf_ref++;
    tbl_policy_map_config_set_policy_map_config_field(pst_policy_map_config, TBL_POLICY_MAP_CONFIG_FLD_INTF_REF);

    if (tbl_interface_get_interface_by_name(GLB_IFNAME_CPU_PREFIX) == pst_interface)
    {
        sal_strcpy(p_copp_db_glb->copp_pname, policy_map_name);
        tbl_copp_cfg_set_copp_cfg_field(p_copp_db_glb, TBL_COPP_CFG_FLD_COPP_PNAME);
        p_copp_db_glb->copp_ifidx = pst_interface->ifindex;
        tbl_copp_cfg_set_copp_cfg_field(p_copp_db_glb, TBL_COPP_CFG_FLD_COPP_IFIDX);
    }
    return PM_E_NONE;
}

int32
policy_map_detach_interface(char *policy_map_name, char *interface_name, bool direct_in)
{
    tbl_interface_t *pst_interface = NULL;
    tbl_policy_map_config_t *pst_policy_map_config   = NULL;
    tbl_policy_map_config_key_t policy_map_config_key;
    tbl_copp_cfg_t *p_copp_db_glb = tbl_copp_cfg_get_copp_cfg();
    int32 rc = PM_E_NONE;

    pst_interface = tbl_interface_get_interface_by_name(interface_name);
    if (NULL == pst_interface)
    {
        return PM_E_NOT_EXIST;
    }

    sal_memset(&policy_map_config_key, 0, sizeof(tbl_policy_map_config_key_t));
    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_strcpy(policy_map_config_key.name, policy_map_name);
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config(&policy_map_config_key);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }

    acl_policy_operation(interface_name, direct_in, policy_map_name, NULL, NULL, 0, GLB_ACL_POLICY_OP_IF_PMAP, GLB_ACL_POLICY_OP_ACTION_DEL);
    /* added by hansf for 2018/10/09 for policer resource del*/
    PM_E_RETURN(acl_policy_attach_or_detach_interface_policer_res_op(policy_map_name, direct_in, ACL_POLICY_POLICER_RES_DEL));
    
    if (direct_in)
    {
        sal_memset(pst_interface->igs_policy_map, 0, sizeof(pst_interface->igs_policy_map));
        tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_IGS_POLICY_MAP);
    }
    else
    {
        sal_memset(pst_interface->egs_policy_map, 0, sizeof(pst_interface->egs_policy_map));
        tbl_interface_set_interface_field(pst_interface, TBL_INTERFACE_FLD_EGS_POLICY_MAP);
    }

    pst_policy_map_config->intf_ref--;
    tbl_policy_map_config_set_policy_map_config_field(pst_policy_map_config, TBL_POLICY_MAP_CONFIG_FLD_INTF_REF);
    if (tbl_interface_get_interface_by_name(GLB_IFNAME_CPU_PREFIX) == pst_interface)
    {
        sal_strcpy(p_copp_db_glb->copp_pname, "");
        tbl_copp_cfg_set_copp_cfg_field(p_copp_db_glb, TBL_COPP_CFG_FLD_COPP_PNAME);
        p_copp_db_glb->copp_ifidx = 0;
        tbl_copp_cfg_set_copp_cfg_field(p_copp_db_glb, TBL_COPP_CFG_FLD_COPP_IFIDX);
    }
    return PM_E_NONE;
}

int32
policy_map_config_add_policy_map(char *policy_map_name)
{
    tbl_policy_map_config_t *pst_policy_map_config = NULL;
    tbl_policy_map_config_t policy_map_config;
    uint32 policy_map_config_cnt = 0;
    
    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }

    /*modified by yejl for bug 54077, 2019-10-22*/
    sal_memset(&policy_map_config, 0, sizeof(policy_map_config));
    sal_strcpy(policy_map_config.key.name, policy_map_name);
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config(&(policy_map_config.key));
    if (NULL != pst_policy_map_config)
    {
        return PM_E_NONE;
    }

    policy_map_config_cnt = tbl_policy_map_config_get_policy_map_config_count();
    if (policy_map_config_cnt >= ACL_POLICY_MAP_CONFIG_NUMBER)
    {
        return PM_E_RESOURCE_FULL;
    }
    /*ended*/

    return tbl_policy_map_config_add_policy_map_config(&policy_map_config); 
}

int32
acl_policy_operation_policy_map_del(char* policy_map_name)
{
    tbl_interface_master_t *p_interface_master = tbl_interface_get_master();
    ctclib_slistnode_t *interface_listnode = NULL;
    ctclib_slistnode_t *interface_next = NULL;
    tbl_interface_t *p_db_if = NULL;

    tbl_class_in_policy_config_master_t *p_class_in_policy_master = tbl_class_in_policy_config_get_master();
    ctclib_slistnode_t *class_in_policy_listnode = NULL;
    ctclib_slistnode_t *class_in_policy_next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, policy_map_name))
        {
            class_detach_policy(policy_map_name, p_db_class_in_policy_config->key.class_name);
        }
    }

    CTCLIB_SLIST_LOOP_DEL(p_interface_master->if_list, p_db_if, interface_listnode, interface_next)
    {
        if(0 == sal_strcmp(p_db_if->igs_policy_map, policy_map_name))
        {
            policy_map_detach_interface(policy_map_name, p_db_if->key.name, TRUE);
        }
        if(0 == sal_strcmp(p_db_if->egs_policy_map, policy_map_name))
        {
            policy_map_detach_interface(policy_map_name, p_db_if->key.name, FALSE);
        }
    }

    return PM_E_NONE;
}

int32
policy_map_config_del_policy_map(char *policy_map_name)
{
    tbl_policy_map_config_t *pst_policy_map_config = NULL;
    tbl_policy_map_config_key_t policy_map_config_key;

    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&policy_map_config_key, 0, sizeof(policy_map_config_key));
    sal_strcpy(policy_map_config_key.name, policy_map_name);
    pst_policy_map_config = tbl_policy_map_config_get_policy_map_config(&policy_map_config_key);
    if (NULL == pst_policy_map_config)
    {
        return PM_E_PMAP_NOT_EXIST;
    }

    acl_policy_operation_policy_map_del(policy_map_name);

    return tbl_policy_map_config_del_policy_map_config(&policy_map_config_key); 
}

int32
class_map_config_add_class_map(char *class_map_name)
{
    tbl_class_map_config_t *pst_class_map_config = NULL;
    tbl_class_map_config_t class_map_config;
    uint32 class_map_config_cnt = 0;

    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }

    /*modified by yejl for bug 54077, 2019-10-22*/
    sal_memset(&class_map_config, 0, sizeof(class_map_config));
    sal_strcpy(class_map_config.key.name, class_map_name);
    pst_class_map_config = tbl_class_map_config_get_class_map_config(&(class_map_config.key));
    if (NULL != pst_class_map_config)
    {
        return PM_E_NONE;
    }

    class_map_config_cnt = tbl_class_map_config_get_class_map_config_count();
    if (class_map_config_cnt >= ACL_CLASS_MAP_CONFIG_NUMBER)
    {
        return PM_E_RESOURCE_FULL;
    }
    /*ended */

    return tbl_class_map_config_add_class_map_config(&class_map_config); 
}

int32
acl_policy_operation_class_map_del(char* class_map_name)
{
    tbl_class_in_policy_config_master_t *p_class_in_policy_master = tbl_class_in_policy_config_get_master();
    ctclib_slistnode_t *class_in_policy_listnode = NULL;
    ctclib_slistnode_t *class_in_policy_next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
    {
        if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name , class_map_name))
        {
            acl_detach_class(class_map_name, p_db_acl_in_class_config->key.acl_name); 
        }
    }

    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.class_name, class_map_name))
        {
            class_detach_policy(p_db_class_in_policy_config->key.policy_name, class_map_name);
        }
    }

    return PM_E_NONE;
}

int32
class_map_config_del_class_map(char *class_map_name)
{
    tbl_class_map_config_t *pst_class_map_config = NULL;
    tbl_class_map_config_key_t class_map_config_key;

    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&class_map_config_key, 0, sizeof(class_map_config_key));
    sal_strcpy(class_map_config_key.name, class_map_name);
    pst_class_map_config = tbl_class_map_config_get_class_map_config(&class_map_config_key);
    if (NULL == pst_class_map_config)
    {
        return PM_E_CMAP_NOT_EXIST;
    }

    acl_policy_operation_class_map_del(class_map_name);

    return tbl_class_map_config_del_class_map_config(&class_map_config_key); 
}

int32
acl_policy_operation_acl_group_del(char* acl_name)
{
    tbl_ace_config_master_t *p_ace_config_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *ace_config_listnode = NULL;
    ctclib_slistnode_t *ace_config_next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_ace_config_master->ace_config_list, p_db_ace_config, ace_config_listnode, ace_config_next)
    {
        if(0 == sal_strcmp(p_db_ace_config->key.acl_name, acl_name))
        {
            acl_config_del_ace(acl_name, p_db_ace_config->key.seq_no, NULL, 0, FALSE);
        }
    }

    CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
    {
        if(0 == sal_strcmp(p_db_acl_in_class_config->key.acl_name, acl_name))
        {
            acl_detach_class(p_db_acl_in_class_config->key.class_name, acl_name); 
        }
    }
    return PM_E_NONE;
}

int32
acl_entry_action_update_by_class_map_action(uint64 acl_id, tbl_class_map_action_config_t* pst_class_map_action_config, uint32 field_id)
{
    tbl_acl_entry_action_t*  pst_acl_entry_action = NULL;
    tbl_acl_entry_action_key_t acl_entry_action_key;

    sal_memset(&acl_entry_action_key, 0x0, sizeof(tbl_acl_entry_action_key_t));
    acl_entry_action_key.aclid = acl_id;
    pst_acl_entry_action = tbl_acl_entry_action_get_acl_entry_action(&acl_entry_action_key);
    if(NULL == pst_acl_entry_action)
    {
        return PM_E_NOT_EXIST;
    }
    switch(field_id)
    {
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC:
            /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
//          <<<<<<<< OLD
//            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_TC))
//            {
//                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_TC);
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
//                pst_acl_entry_action->tc = pst_class_map_action_config->new_tc;
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_TC);
//            }
//            else
//            {
//                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_TC);
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);        
//                pst_acl_entry_action->tc = pst_class_map_action_config->new_tc;
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_TC);
//            }
//          >>>>>>>> NEW
            pst_acl_entry_action->tc = pst_class_map_action_config->new_tc;
//          ======== END
            /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_POLICER);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                sal_strcpy(pst_acl_entry_action->policer, pst_class_map_action_config->policer);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_POLICER);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_POLICER);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                sal_memset(pst_acl_entry_action->policer, 0x0, POLICER_NAME_SIZE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_POLICER);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_COLOR))
            {
                /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
//              <<<<<<<< OLD
//                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_COLOR);
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
//                pst_acl_entry_action->color = pst_class_map_action_config->new_color;
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_COLOR);
//              >>>>>>>> NEW
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_COLOR);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);

                pst_acl_entry_action->color = pst_class_map_action_config->new_color;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_COLOR);
//              ======== END
                /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
            }
            else
            {
                /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
//              <<<<<<<< OLD
//                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_COLOR);
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);        
//                pst_acl_entry_action->color = pst_class_map_action_config->new_color;
//                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_COLOR);
//              >>>>>>>> NEW
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_COLOR);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);        

                pst_acl_entry_action->color = pst_class_map_action_config->new_color;
                pst_acl_entry_action->tc = pst_class_map_action_config->new_tc;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_COLOR);
//              ======== END
                /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_DSCP))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_DSCP);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->dscp = pst_class_map_action_config->new_dscp;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_DSCP);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_DSCP);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);        
                pst_acl_entry_action->dscp = pst_class_map_action_config->new_dscp;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_DSCP);
            }
            break;
         case TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->session_id = pst_class_map_action_config->session_id;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_MIRROR_SESSION);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->session_id = 0;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_SESSION_ID);
            }
            break;
         case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SVLAN_ID);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_svlan_id = pst_class_map_action_config->new_svlan_id;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SVLAN_ID);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_svlan_id = 0;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_SVLAN_ID);
            }
            break;
         case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CVLAN_ID);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_cvlan_id = pst_class_map_action_config->new_cvlan_id;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CVLAN_ID);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_cvlan_id = 0;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_CVLAN_ID);
            }
            break;
         case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_scos = pst_class_map_action_config->new_scos;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_scos = 0;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_SCOS);
            }
            break;
         case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_ccos = pst_class_map_action_config->new_ccos;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->new_ccos = 0;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_NEW_CCOS);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX:
            if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
            {
                GLB_SET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_REDIRECT_VALUE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->redirect_port_ifindex = pst_class_map_action_config->redirect_port_ifindex;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX);
            }
            else
            {
                GLB_UNSET_FLAG(pst_acl_entry_action->action_flag, GLB_ACL_SET_REDIRECT_VALUE);
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_ACTION_FLAG);
                pst_acl_entry_action->redirect_port_ifindex  = 0;
                tbl_acl_entry_action_set_acl_entry_action_field(pst_acl_entry_action, TBL_ACL_ENTRY_ACTION_FLD_REDIRECT_PORT_IFINDEX);
            }
        default:
            break;
    }

    return PM_E_NONE;
}

int32
class_map_action_set_redirect_port(tbl_class_map_action_config_t* pst_class_map_action_config, char* value, cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    
    if ('\0' != value[0])
    {
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
        {
            CFG_CONFLICT_RET("Redirect had been configured");
        }
        if ('\0' != pst_class_map_action_config->red_unexist_agg[0] &&
            0 != sal_strcmp(pst_class_map_action_config->red_unexist_agg, value))
        {
            CFG_CONFLICT_RET("Redirect had been configured");
        }
        p_db_if = tbl_interface_get_interface_by_name(value);
        if (NULL == p_db_if)
        {   
            /*modified by huwx for bug 44959*/
            if (0 == sal_strcmp(pst_class_map_action_config->red_unexist_agg, value))
            {
                return PM_E_NONE;
            }
            if (0 == sal_memcmp(value, GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN))
            {
                sal_strcpy(pst_class_map_action_config->red_unexist_agg, value);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG);
                return PM_E_NONE; 
            }
            else
            {
                CFG_CONFLICT_RET("Interface not exist");
            }
        }
        /*modified by yejl for bug 54245, 2019-11-05*/
        /*
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            CFG_CONFLICT_RET("Interface %s is a LAG member port", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        */
        GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE);
        pst_class_map_action_config->redirect_port_ifindex = p_db_if->ifindex;
        tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
        tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX);
    }
         
    return PM_E_NONE;   
}

int32
acl_api_agg_set_redirect_cb(tbl_interface_t *p_agg_if)
{
    tbl_class_map_action_config_master_t *p_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_class_map_action_config_t *p_db_config = NULL;
    tbl_interface_master_t *p_if_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    tbl_ace_config_master_t *p_ace_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *ace_listnode = NULL;
    ctclib_slistnode_t *ace_next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;
    uint64 acl_id = 0;
    uint64 type = 0;
    
    CTCLIB_SLIST_LOOP(p_master->class_map_action_config_list, p_db_config, listnode)
    {
        if (0 == sal_strcmp(p_db_config->red_unexist_agg, p_agg_if->key.name))
        {
            GLB_SET_FLAG(p_db_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE);
            p_db_config->redirect_port_ifindex = p_agg_if->ifindex;
            tbl_class_map_action_config_set_class_map_action_config_field(p_db_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
            tbl_class_map_action_config_set_class_map_action_config_field(p_db_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX);

            CTCLIB_SLIST_LOOP_DEL(p_if_master->if_list, p_db_if, if_listnode, if_next)
            {
                if(0 == sal_strcmp(p_db_if->igs_policy_map, p_db_config->key.policy_name))
                {
                    p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(p_db_config->key.policy_name, p_db_config->key.class_name);
                    CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
                    {
                        if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                        {
                            CTCLIB_SLIST_LOOP_DEL(p_ace_master->ace_config_list, p_db_ace_config, ace_listnode, ace_next)
                            {
                                if(0 == sal_strcmp(p_db_ace_config->key.acl_name, p_db_acl_in_class_config->key.acl_name))
                                {
                                    if (p_db_ace_config->is_ipv6 || p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_IPV6;
                                    }
                                    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_MAC;
                                    }
                                    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_NORMAL;
                                    }
                                    acl_id = MAKE_ACL_ID(p_db_ace_config->key.seq_no, 1, p_db_if->ifindex, p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, type);
                                    acl_entry_action_update_by_class_map_action(acl_id, p_db_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return PM_E_NONE; 
}

int32
class_map_action_unset_redirect_port(tbl_class_map_action_config_t* pst_class_map_action_config)
{
    GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE);
    pst_class_map_action_config->redirect_port_ifindex = 0;
    sal_memset(pst_class_map_action_config->red_unexist_agg, 0x0, sizeof(pst_class_map_action_config->red_unexist_agg));
    tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_RED_UNEXIST_AGG);
    tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
    tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX);

    return PM_E_NONE;   
}

int32
acl_api_if_del_cb(tbl_interface_t *p_db_if)
{
    tbl_class_map_action_config_master_t *p_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_class_map_action_config_t *p_db_config = NULL;

    /*modified by yejl for bug 54245, 2019-11-05*/
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        if (NULL == p_master)
        {
            return PM_E_NONE;
        }

        CTCLIB_SLIST_LOOP(p_master->class_map_action_config_list, p_db_config, listnode)
        {
            if (p_db_config->redirect_port_ifindex)
            {
                class_map_action_unset_redirect_port(p_db_config);
            }
        }
    }
    
    return PM_E_NONE;
}

int32
class_map_action_show(cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    sal_memset(&args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    args.argv[0] = &field;
    args.argv[1] = fp;

    /*dump all class_map_action*/       
    rc = tbl_class_map_action_config_iterate((TBL_ITER_CB_FUNC)tbl_class_map_action_config_dump_one, &args);
    sal_fclose(fp);

    return rc;
}

int32
class_map_action_update(char *policy_map_name, char *class_map_name, uint32 field_id, char* value, cfg_cmd_para_t *para)
{
    tbl_class_map_action_config_t* pst_class_map_action_config = NULL;
    tbl_class_map_action_config_t class_map_action_config;
    uint32 direct_in = 0;
    uint64 acl_id = 0;
    uint32 tc = 0;
    uint64 type = 0;
    int32 rc = PM_E_NONE;

    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *if_listnode = NULL;
    ctclib_slistnode_t *if_next = NULL;
    tbl_interface_t *p_db_if = NULL;
    
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    tbl_ace_config_master_t *p_ace_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *ace_listnode = NULL;
    ctclib_slistnode_t *ace_next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;

    tbl_qos_policer_profile_t* pst_qos_policer_profile = NULL;

    int32 ret = PM_E_NONE;

    if (check_name_character_and_len(policy_map_name, PMAP_NAME_SIZE) != 0)
    {
        return PM_E_NOT_PERMIT;
    }
    if (check_name_character_and_len(class_map_name, CMAP_NAME_SIZE) != 0)
    {
        return PM_E_INVALID_PARAM;
    }

    if (NULL == value)
    {
        return PM_E_INVALID_PARAM;
    }

    /*get class_map_action_config tbl*/
    pst_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config_by_name(policy_map_name, class_map_name);
    if(NULL == pst_class_map_action_config)
    {
        sal_memset(&class_map_action_config, 0x0, sizeof(tbl_class_map_action_config_t));
        sal_strcpy(class_map_action_config.key.policy_name, policy_map_name);
        sal_strcpy(class_map_action_config.key.class_name, class_map_name);
        
        tbl_class_map_action_config_add_class_map_action_config(&class_map_action_config);

        pst_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config_by_name(policy_map_name, class_map_name);
        if(NULL == pst_class_map_action_config)
        {
            return PM_E_FAIL;
        }
    }

    if((TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC == field_id) ||
    //    (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS == field_id) ||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX == field_id))
    {
        ret = acl_policy_action_update_action_check(policy_map_name);
        if(ret != PM_E_NONE)
        {
            return ret;
        }
    }

    /*added by huwx for copp action check, only support policer update*/
    if((TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER == field_id) ||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC == field_id) ||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP == field_id)||
 //       (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR == field_id)|| /*added by yejl for bug 51468, 2019-03-29*/
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS == field_id)||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS == field_id) ||
        (TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX == field_id))
    {
        ret = acl_copp_policy_action_update_action_check(policy_map_name);
        if(ret != PM_E_NONE)
        {
            return ret;
        }
    }

    /*update class_map_action_config tbl*/
    switch(field_id)
    {
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC:
            if('\0' != value[0])
            {
                tc = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_TC);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_tc = tc;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_TC);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_tc = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER:
            if('\0' != value[0])
            {
                if('\0' != pst_class_map_action_config->policer[0]) /* policer has been configured */
                {
                    CFG_INVALID_PARAM_RET("Policer had been configured!");
                }
                pst_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(value);
                if(!pst_qos_policer_profile)
                {
                    CFG_INVALID_PARAM_RET("Policer profile does not exist!");
                }

                /*added by hansf 2018/10/09 for policer resource check */
                PM_E_RETURN(acl_policy_class_action_policer_res_op(policy_map_name, class_map_name, value, ACL_POLICY_POLICER_RES_CHECK));
                PM_E_RETURN(acl_policy_class_action_policer_res_op(policy_map_name, class_map_name, value, ACL_POLICY_POLICER_RES_ADD));
                
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                sal_strcpy(pst_class_map_action_config->policer, value);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER);
                pst_qos_policer_profile->ref_cnt ++;
                tbl_qos_policer_profile_set_qos_policer_profile_field(pst_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT);
            }
            else
            {
                if(!GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
                {
                    CFG_INVALID_PARAM_RET("No policer applied in the policy map");
                }
                pst_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(pst_class_map_action_config->policer);
                if(!pst_qos_policer_profile)
                {
                    CFG_INVALID_PARAM_RET("Policer profile does not exist");
                }
                
                /*added by hansf 2018/10/09 for policer resource del */
                PM_E_RETURN(acl_policy_class_action_policer_res_op(policy_map_name, class_map_name, pst_class_map_action_config->policer, ACL_POLICY_POLICER_RES_DEL));
                
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                sal_memset(pst_class_map_action_config->policer, 0x0, POLICER_NAME_SIZE);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_POLICER);
                pst_qos_policer_profile->ref_cnt --;
                tbl_qos_policer_profile_set_qos_policer_profile_field(pst_qos_policer_profile, TBL_QOS_POLICER_PROFILE_FLD_REF_CNT);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR:
            if('\0' != value[0])
            {
                pst_class_map_action_config->new_color = qos_mgr_map_str2color(value); /*glb_qos_color_t*/
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_COLOR);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_COLOR);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_color = GLB_QOS_COLOR_NONE;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP:
            if('\0' != value[0])
            {
                pst_class_map_action_config->new_dscp = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_DSCP);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_DSCP);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_dscp = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID:
            if('\0' != value[0])
            {
                pst_class_map_action_config->session_id = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID);
            }
            else
            {
                if(!GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION))
                {
                    CFG_INVALID_PARAM_RET("No mirror session applied in the policy-map");
                }
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->session_id = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID:
            if('\0' != value[0])
            {
                pst_class_map_action_config->new_svlan_id = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_svlan_id = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID:
            if('\0' != value[0])
            {
                pst_class_map_action_config->new_cvlan_id = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_cvlan_id = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS:
            if('\0' != value[0])
            {
                pst_class_map_action_config->new_scos = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_scos = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS:
            if('\0' != value[0])
            {
                pst_class_map_action_config->new_ccos = sal_atoi(value);
                GLB_SET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS);
            }
            else
            {
                GLB_UNSET_FLAG(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE);
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_ACTION_FLAG);
                pst_class_map_action_config->new_ccos = 0;
                tbl_class_map_action_config_set_class_map_action_config_field(pst_class_map_action_config, TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS);
            }
            break;
        case TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX:
            if ('\0' != value[0])
            {
                PM_E_RETURN(class_map_action_set_redirect_port(pst_class_map_action_config, value, para));
            }
            else
            {
                PM_E_RETURN(class_map_action_unset_redirect_port(pst_class_map_action_config));
            }
            break;
        default:
            break;
    }

    /*get tbl_id, and update acl entry action*/
    CTCLIB_SLIST_LOOP_DEL(p_master->if_list, p_db_if, if_listnode, if_next)
    {
        if(0 == sal_strcmp(p_db_if->igs_policy_map, policy_map_name))
        {
            direct_in = 1;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    CTCLIB_SLIST_LOOP_DEL(p_ace_master->ace_config_list, p_db_ace_config, ace_listnode, ace_next)
                    {
                        if(0 == sal_strcmp(p_db_ace_config->key.acl_name, p_db_acl_in_class_config->key.acl_name))
                        {
                            if (p_db_ace_config->is_ipv6 || p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
                            {
                                type = GLB_ACL_CONFIG_TYPE_IPV6;
                            }
                            else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
                            {
                                type = GLB_ACL_CONFIG_TYPE_MAC;
                            }
                            else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
                            {
                                type = GLB_ACL_CONFIG_TYPE_NORMAL;
                            }
                            acl_id = MAKE_ACL_ID(p_db_ace_config->key.seq_no, direct_in, p_db_if->ifindex, p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, type);
                            acl_entry_action_update_by_class_map_action(acl_id, pst_class_map_action_config, field_id);
                        }
                    }
                }
            }
        }

        if(0 == sal_strcmp(p_db_if->egs_policy_map, policy_map_name))
        {
            direct_in = 0;
            p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config_by_name(policy_map_name, class_map_name);
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                {
                    CTCLIB_SLIST_LOOP_DEL(p_ace_master->ace_config_list, p_db_ace_config, ace_listnode, ace_next)
                    {
                        if(0 == sal_strcmp(p_db_ace_config->key.acl_name, p_db_acl_in_class_config->key.acl_name))
                        {
                            if (p_db_ace_config->is_ipv6 || p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
                            {
                                type = GLB_ACL_CONFIG_TYPE_IPV6;
                            }
                            else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
                            {
                                type = GLB_ACL_CONFIG_TYPE_MAC;
                            }
                            else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
                            {
                                type = GLB_ACL_CONFIG_TYPE_NORMAL;
                            }
                            acl_id = MAKE_ACL_ID(p_db_ace_config->key.seq_no, direct_in, p_db_if->ifindex, p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, type);
                            acl_entry_action_update_by_class_map_action(acl_id, pst_class_map_action_config, field_id);
                        }
                    }
                }
            }
        }
    }
    
    return PM_E_NONE;
}

int32
acl_policy_build_qos_policer_profile(FILE* fp, cfg_cmd_para_t *para, tbl_qos_policer_profile_t *p_qos_policer_profile_db)
{
    char* mode_str[] = 
    {
        "rfc2697",
        "rfc2698",
        "rfc4115",
        "bwp"
    };
    char* color_mode_str[] = 
    {
        "color-blind",
        "color-aware"
    };   
    sal_fprintf(fp," policer-profile %s (", p_qos_policer_profile_db->name);
    sal_fprintf(fp," mode %s", mode_str[p_qos_policer_profile_db->algorithm_mode]);
    if (p_qos_policer_profile_db->is_pps)
    {
        sal_fprintf(fp," pps");
    }
    if (GLB_QOS_POLICER_MODE_RFC4115 == p_qos_policer_profile_db->algorithm_mode)
    {
        sal_fprintf(fp," %s cir %"PRIu64" cbs %d eir %"PRIu64" ebs %d ", color_mode_str[p_qos_policer_profile_db->color_mode], p_qos_policer_profile_db->cir,
            p_qos_policer_profile_db->cbs, p_qos_policer_profile_db->eir_or_pir, p_qos_policer_profile_db->ebs_or_pbs);

        if (GLB_FLAG_ISSET(p_qos_policer_profile_db->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
        {
            sal_fprintf(fp,"stats");
        }
        sal_fprintf(fp,")\n");
    }
    if (GLB_QOS_POLICER_MODE_RFC2697 == p_qos_policer_profile_db->algorithm_mode)
    {
        sal_fprintf(fp," %s cir %"PRIu64" cbs %d ebs %d ", color_mode_str[p_qos_policer_profile_db->color_mode], p_qos_policer_profile_db->cir,
            p_qos_policer_profile_db->cbs, p_qos_policer_profile_db->ebs_or_pbs);
        if (GLB_FLAG_ISSET(p_qos_policer_profile_db->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
        {
            sal_fprintf(fp,"stats");
        }
        sal_fprintf(fp,")\n");
    }

    return PM_E_NONE;
}


int32
acl_policy_show_acl_policy(FILE* fp, char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t *pst_interface   = NULL;

    tbl_class_in_policy_config_master_t *p_class_in_policy_master = tbl_class_in_policy_config_get_master();
    ctclib_slistnode_t *class_in_policy_listnode = NULL;
    ctclib_slistnode_t *class_in_policy_next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;
    tbl_class_in_policy_config_key_t p_class_in_policy_key;


    tbl_acl_in_class_config_master_t *p_acl_in_class_master = tbl_acl_in_class_config_get_master();
    ctclib_slistnode_t *acl_in_class_listnode = NULL;
    ctclib_slistnode_t *acl_in_class_next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    tbl_ace_config_master_t *p_ace_config_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *ace_config_listnode = NULL;
    ctclib_slistnode_t *ace_config_next = NULL;
    tbl_ace_config_t *p_db_ace_config = NULL;

    tbl_fea_acl_policy_action_t* pst_fea_acl_policy_action = NULL;
    tbl_fea_acl_policy_action_key_t fea_acl_policy_action_key;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;

    tbl_iter_args_t args;

    tbl_fea_acl_t* pst_fea_acl = NULL;
    tbl_fea_acl_key_t fea_acl_key;

    uint64 acl_id = 0;
    uint64 class_packet = 0;
    uint64 class_bytes = 0;

    uint32 class_base = FALSE;
    uint32 ace_base = FALSE;
    char class_name[CMAP_NAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 is_by_policy = TRUE;
    uint32 acl_has_print = FALSE; 
    uint64 type = 0;

    (void)class_base;

    sal_memset(&p_class_in_policy_key, 0, sizeof(tbl_class_in_policy_config_key_t));
    sal_memset(class_name, 0, sizeof(class_name));
    
    pst_interface = tbl_interface_get_interface_by_name(argv[0]);
    if (NULL == pst_interface)
    {
        CFG_INVALID_PARAM_RET("Interface %s does not exist!", argv[0]);
    }

    if (!sal_strcmp(argv[2], "ace-based"))
    {
        ace_base = TRUE;
    }
    else
    {
        class_base = TRUE;
    }

    if ('\0' != argv[3][0])
    {
        /*added by yejl to fix bug38867*/
        if (check_name_character_and_len(argv[3], CMAP_NAME_SIZE) != 0)
        {
            CFG_INVALID_PARAM_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        sal_strcpy(class_name, argv[3]);
    }
    if (!sal_strcmp(argv[1], "input"))
    {
        if (0 != sal_strlen(pst_interface->igs_policy_map))
        {
            if('\0' != class_name[0])
            {
                sal_strcpy(p_class_in_policy_key.class_name, class_name);
                sal_strcpy(p_class_in_policy_key.policy_name, pst_interface->igs_policy_map);
                p_db_class_in_policy_config = tbl_class_in_policy_config_get_class_in_policy_config(&p_class_in_policy_key);
                if(!p_db_class_in_policy_config)
                {
                    CFG_INVALID_PARAM_RET("The class map %s does not applied in this policy-map", class_name);
                }
            }
            if (0 != sal_strcmp(pst_interface->key.name, "cpu"))
            {
                sal_fprintf (fp, "Interface: %s\n", IFNAME_ETH2FULL(pst_interface->key.name, ifname_ext));
            }
            sal_fprintf (fp, "Ingress service policy: %s\n", pst_interface->igs_policy_map);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Policy-map does not exist on the interface input direction");
        }
        CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
        {
            if (!sal_strcmp(p_db_class_in_policy_config->key.policy_name, pst_interface->igs_policy_map))
            {
                if ('\0' != argv[3][0])
                {
                    if (sal_strcmp(p_db_class_in_policy_config->key.class_name, class_name))
                    {
                        continue;
                    }
                }
                sal_fprintf(fp, " Class name: %s", p_db_class_in_policy_config->key.class_name);
                if (ace_base)
                {
                    sal_fprintf (fp,"\n");
                }
                CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
                {
                    if (!sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                    {
                            CTCLIB_SLIST_LOOP_DEL(p_ace_config_master->ace_config_list, p_db_ace_config, ace_config_listnode, ace_config_next)
                            {
                                if (!sal_strcmp(p_db_ace_config->key.acl_name, p_db_acl_in_class_config->key.acl_name))
                                {
                                    if (p_db_ace_config->is_ipv6 || p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_IPV6;
                                    }
                                    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_MAC;
                                    }
                                    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_NORMAL;
                                    } 
                                    acl_id = MAKE_ACL_ID(p_db_ace_config->key.seq_no, 1, pst_interface->ifindex, p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, type);
                                    sal_memset(&fea_acl_key, 0x0, sizeof(tbl_fea_acl_key_t));
                                    fea_acl_key.acl_id = acl_id;
                                    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
                                    if (NULL == pst_fea_acl)
                                    {
                                        continue;
                                    }
                                    if (ace_base)
                                    {
                                        sal_memset(&args, 0x0, sizeof(tbl_iter_args_t));
                                        args.argv[0] = fp;
                                        args.argv[1] = p_db_ace_config->key.acl_name;
                                        args.argv[2] = &is_by_policy;
                                        if (!acl_has_print)
                                        {
                                            sal_fprintf (fp, "  access-group: %s\n", p_db_ace_config->key.acl_name);
                                            acl_has_print = TRUE;
                                        }
                                        _acl_show_ace_config(p_db_ace_config, &args);
                                        sal_fprintf (fp," (%"PRIu64" match %"PRIu64" bytes)\n", pst_fea_acl->stats_packet, pst_fea_acl->stats_byte);
                                    }
                                    class_packet += pst_fea_acl->stats_packet;
                                    class_bytes += pst_fea_acl->stats_byte;
                                }
                            }
                            acl_has_print = FALSE;
                    }
                }
                type = 0;
                if (ace_base)
                {
                    sal_fprintf (fp, " total (%"PRIu64" match %"PRIu64" bytes)\n", class_packet, class_bytes);
                    sal_fprintf (fp,"\n");
                }
                else
                {
                    sal_fprintf (fp, "(%"PRIu64" match %"PRIu64" bytes)\n", class_packet, class_bytes);
                    sal_memset(&fea_acl_policy_action_key, 0x0, sizeof(tbl_fea_acl_policy_action_key_t));
                    fea_acl_policy_action_key.acl_policy_action_id = MAKE_ACL_ID(0, 1, pst_interface->ifindex, p_db_class_in_policy_config->class_priority, 0x0ULL, type);
                    pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
                    if (pst_fea_acl_policy_action)
                    {
                        p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(pst_fea_acl_policy_action->policer_name);
                        if (p_db_qos_policer_profile)
                        {
                            acl_policy_build_qos_policer_profile(fp, para, p_db_qos_policer_profile);
                            if (GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
                            {
                                sal_fprintf (fp, "    Total packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->green_packet + pst_fea_acl_policy_action->yellow_packet + pst_fea_acl_policy_action->red_packet, 
                                                                                 pst_fea_acl_policy_action->green_byte + pst_fea_acl_policy_action->yellow_byte + pst_fea_acl_policy_action->red_byte);
                                sal_fprintf (fp, "    Confirm packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->green_packet, pst_fea_acl_policy_action->green_byte);
                                sal_fprintf (fp, "    Exceed packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->yellow_packet, pst_fea_acl_policy_action->yellow_byte);
                                sal_fprintf (fp, "    Violate packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->red_packet, pst_fea_acl_policy_action->red_byte);
                            }
                        }

                    }
                }
                class_packet = 0;
                class_bytes = 0;
            }
        }

    }
    else
    {
        if (0 != sal_strlen(pst_interface->egs_policy_map))
        {
            sal_fprintf (fp, "Interface: %s\n", IFNAME_ETH2FULL(pst_interface->key.name, ifname_ext));
            sal_fprintf (fp, "Egress service policy: %s\n", pst_interface->egs_policy_map);/*modified by yejl for bug 50096, 2018-11-23*/
        }
        else
        {
            CFG_INVALID_PARAM_RET("policy-map does not exist on the interface output direction");
        }
        CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
        {
            if (!sal_strcmp(p_db_class_in_policy_config->key.policy_name, pst_interface->egs_policy_map))
            {
                if ('\0' != argv[3][0])
                {
                    if (sal_strcmp(p_db_class_in_policy_config->key.class_name, class_name))
                    {
                        continue;
                    }
                }
                sal_fprintf (fp, " Class name: %s", p_db_class_in_policy_config->key.class_name);
                if (ace_base)
                {
                    sal_fprintf (fp,"\n");
                }
                CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
                {
                    if (!sal_strcmp(p_db_acl_in_class_config->key.class_name, p_db_class_in_policy_config->key.class_name))
                    {
                            CTCLIB_SLIST_LOOP_DEL(p_ace_config_master->ace_config_list, p_db_ace_config, ace_config_listnode, ace_config_next)
                            {
                                if (!sal_strcmp(p_db_ace_config->key.acl_name, p_db_acl_in_class_config->key.acl_name))
                                {
                                    if (p_db_ace_config->is_ipv6 || p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_IPV6;
                                    }
                                    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_MAC;
                                    }
                                    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_NORMAL)
                                    {
                                        type = GLB_ACL_CONFIG_TYPE_NORMAL;
                                    } 
                                    acl_id = MAKE_ACL_ID(p_db_ace_config->key.seq_no, 0, pst_interface->ifindex, p_db_class_in_policy_config->class_priority, p_db_acl_in_class_config->acl_priority, type);
                                    sal_memset(&fea_acl_key, 0x0, sizeof(tbl_fea_acl_key_t));
                                    fea_acl_key.acl_id = acl_id;
                                    pst_fea_acl = tbl_fea_acl_get_fea_acl(&fea_acl_key);
                                    if (NULL == pst_fea_acl)
                                    {
                                        continue;
                                    }
                                    if (ace_base)
                                    {
                                        sal_memset(&args, 0x0, sizeof(tbl_iter_args_t));
                                        args.argv[0] = fp;
                                        args.argv[1] = p_db_ace_config->key.acl_name;
                                        args.argv[2] = &is_by_policy;
                                        if (!acl_has_print)
                                        {
                                            sal_fprintf (fp, "  access-group: %s\n", p_db_ace_config->key.acl_name);
                                            acl_has_print = TRUE;
                                        }
                                        _acl_show_ace_config(p_db_ace_config, &args);
                                        sal_fprintf (fp," (%"PRIu64" match %"PRIu64" bytes)\n", pst_fea_acl->stats_packet, pst_fea_acl->stats_byte);
                                    }
                                    class_packet += pst_fea_acl->stats_packet;
                                    class_bytes += pst_fea_acl->stats_byte;
                                }
                            }
                            acl_has_print = FALSE;
                    }
                }
                type = 0;
                if (ace_base)
                {
                    sal_fprintf (fp, " total (%"PRIu64" match %"PRIu64" bytes)\n", class_packet, class_bytes);
                    sal_fprintf (fp,"\n");
                }
                else
                {
                    sal_fprintf (fp, "(%"PRIu64" match %"PRIu64" bytes)\n", class_packet, class_bytes);
                    sal_memset(&fea_acl_policy_action_key, 0x0, sizeof(tbl_fea_acl_policy_action_key_t));
                    fea_acl_policy_action_key.acl_policy_action_id = MAKE_ACL_ID(0, 0, pst_interface->ifindex, p_db_class_in_policy_config->class_priority, 0x0ULL, type);
                    pst_fea_acl_policy_action = tbl_fea_acl_policy_action_get_fea_acl_policy_action(&fea_acl_policy_action_key);
                    if (pst_fea_acl_policy_action)
                    {
                        p_db_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(pst_fea_acl_policy_action->policer_name);
                        if (p_db_qos_policer_profile)
                        {
                            acl_policy_build_qos_policer_profile(fp, para, p_db_qos_policer_profile);
                            if (GLB_FLAG_ISSET(p_db_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
                            {
                                sal_fprintf (fp, "    Total packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->green_packet + pst_fea_acl_policy_action->yellow_packet + pst_fea_acl_policy_action->red_packet, 
                                                                                 pst_fea_acl_policy_action->green_byte + pst_fea_acl_policy_action->yellow_byte + pst_fea_acl_policy_action->red_byte);
                                sal_fprintf (fp, "    Confirm packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->green_packet, pst_fea_acl_policy_action->green_byte);
                                sal_fprintf (fp, "    Exceed packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->yellow_packet, pst_fea_acl_policy_action->yellow_byte);
                                sal_fprintf (fp, "    Violate packets %"PRIu64", bytes %"PRIu64"\n", pst_fea_acl_policy_action->red_packet, pst_fea_acl_policy_action->red_byte);
                            }
                        }

                    }
                }
                class_packet = 0;
                class_bytes = 0;
            }
        }

    }
    return PM_E_NONE;
}

int32
_acl_show_class_in_policy_config(tbl_class_in_policy_config_t *p_class_in_policy_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    char *policy_name = pargs->argv[1];
    tbl_class_map_action_config_t* pst_class_map_action_config = NULL;
    char *color_str[] = {"none", "red", "yellow", "green"};
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t* p_interface = NULL;

    if (0 != sal_strcmp(p_class_in_policy_config->key.policy_name, policy_name))
    {
        return PM_E_NONE;
    }

    sal_fprintf (cfg_fp, " class %s\n", p_class_in_policy_config->key.class_name);

    pst_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config_by_name(p_class_in_policy_config->key.policy_name, p_class_in_policy_config->key.class_name);
    if(NULL != pst_class_map_action_config)
    {
        /*modified by yejl for bug 49954, 2018-11-12*/
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION))
        {
            sal_fprintf (cfg_fp, "  monitor to session %d\n", pst_class_map_action_config->session_id);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
        {
            sal_fprintf (cfg_fp, "  policer %s\n", pst_class_map_action_config->policer);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
        {
            sal_fprintf (cfg_fp, "  set ccos %d\n", pst_class_map_action_config->new_ccos);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
        {
            sal_fprintf (cfg_fp, "  set cvid %d\n", pst_class_map_action_config->new_cvlan_id);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_DSCP))
        {
            sal_fprintf (cfg_fp, "  set dscp %d\n", pst_class_map_action_config->new_dscp);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
        {
            sal_fprintf (cfg_fp, "  set scos %d\n", pst_class_map_action_config->new_scos);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
        {
            sal_fprintf (cfg_fp, "  set svid %d\n", pst_class_map_action_config->new_svlan_id);
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_COLOR))
        {
            sal_fprintf (cfg_fp, "  set tc %d color %s\n", pst_class_map_action_config->new_tc, color_str[pst_class_map_action_config->new_color]);
        } 
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
        {
            p_interface = tbl_interface_get_interface_by_ifindex(pst_class_map_action_config->redirect_port_ifindex);
            if (p_interface)
            {
                IFNAME_ETH2FULL(p_interface->key.name, ifname_ext);
                sal_fprintf (cfg_fp, "  redirect %s\n", ifname_ext);
            }
        }
        else if ('\0' != pst_class_map_action_config->red_unexist_agg[0])
        {
            sal_fprintf (cfg_fp, "  redirect %s\n", pst_class_map_action_config->red_unexist_agg);
        }
    }
    return PM_E_NONE;
}

int32
_acl_show_policy_map(FILE* fp, tbl_policy_map_config_t *p_pmap_config)
{
    FILE *cfg_fp = fp;
    tbl_iter_args_t args;

    sal_fprintf (cfg_fp, "policy-map %s\n", p_pmap_config->key.name);

    if (0 != p_pmap_config->class_ref)
    {
        args.argv[0] = (void *)cfg_fp;
        args.argv[1] = (void *)(p_pmap_config->key.name);
        tbl_class_in_policy_config_iterate((TBL_ITER_CB_FUNC)_acl_show_class_in_policy_config, &args);
    }

    return PM_E_NONE;
}

int32
acl_policy_show_policy_map(tbl_policy_map_config_t *p_db_pmap_config, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_policy_map_config_master_t *p_master = tbl_policy_map_config_get_master();
    ctclib_slistnode_t *listnode = NULL;

    if (p_db_pmap_config)
    {
        _acl_show_policy_map(fp, p_db_pmap_config);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->policy_map_config_list , p_db_pmap_config, listnode)
        {
            _acl_show_policy_map(fp, p_db_pmap_config);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
_acl_show_acl_in_class_config(tbl_acl_in_class_config_t *p_acl_in_class_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    char *class_name = pargs->argv[1];;

    if (0 != sal_strcmp(p_acl_in_class_config->key.class_name, class_name))
    {
        return PM_E_NONE;
    }

    sal_fprintf (cfg_fp, " match access-list %s\n", p_acl_in_class_config->key.acl_name);
    return PM_E_NONE;
}

int32
_acl_show_class_map(FILE* fp, tbl_class_map_config_t *p_cmap_config)
{
    FILE *cfg_fp = fp;
    tbl_iter_args_t args;

    sal_fprintf (cfg_fp, "class-map %s\n", p_cmap_config->key.name);

    if (0 != p_cmap_config->acl_ref)
    {
        args.argv[0] = (void *)cfg_fp;
        args.argv[1] = (void *)(p_cmap_config->key.name);
        tbl_acl_in_class_config_iterate((TBL_ITER_CB_FUNC)_acl_show_acl_in_class_config, &args);
    }

    return PM_E_NONE;
}

int32
acl_policy_show_class_map(tbl_class_map_config_t *p_db_cmap_config, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    tbl_class_map_config_master_t *p_master = tbl_class_map_config_get_master();
    ctclib_slistnode_t *listnode = NULL;

    if (p_db_cmap_config)
    {
        _acl_show_class_map(fp, p_db_cmap_config);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->class_map_config_list , p_db_cmap_config, listnode)
        {
            _acl_show_class_map(fp, p_db_cmap_config);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

static int32
acl_policy_agg_destroy(tbl_interface_t *p_db_agg_if, tbl_interface_t *p_db_mem_if, void *p_param)
{
    if (0 != sal_strlen(p_db_agg_if->igs_policy_map))
    {
        acl_policy_operation_if_pmap_del( p_db_agg_if->key.name, TRUE, p_db_agg_if->igs_policy_map);
    }

    if (0 != sal_strlen(p_db_agg_if->egs_policy_map))
    {
        acl_policy_operation_if_pmap_del( p_db_agg_if->key.name, FALSE, p_db_agg_if->egs_policy_map);
    }

    return PM_E_NONE;
}

int32
_acl_policy_clear_statistics_by_ace(uint64 class_priority, uint64 acl_priority, tbl_interface_t *p_tbl_if, tbl_ace_config_t *p_db_ace_config, bool is_input)
{
    tbl_fea_acl_key_t       p_fea_acl_key;
    tbl_fea_acl_t          *p_fea_acl = NULL;
    tbl_clear_acl_policy_t *p_clear_acl_policy;

    uint64 acl_id = 0;
    uint64 type = 0;

    /* get fea_acl table */
    if (p_db_ace_config->is_ipv6 || p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_IPV6)
    {
        type = GLB_ACL_CONFIG_TYPE_IPV6;
    }
    else if (p_db_ace_config->acl_type == GLB_ACL_CONFIG_TYPE_MAC)
    {
        type = GLB_ACL_CONFIG_TYPE_MAC;
    }
    else
    {
        type = GLB_ACL_CONFIG_TYPE_NORMAL;
    }
    acl_id = MAKE_ACL_ID(p_db_ace_config->key.seq_no, (is_input ? 1 : 0), p_tbl_if->ifindex, class_priority, acl_priority, type);
    sal_memset(&p_fea_acl_key, 0x0, sizeof(tbl_acl_entry_key_t));
    p_fea_acl_key.acl_id = acl_id;
    p_fea_acl = tbl_fea_acl_get_fea_acl(&p_fea_acl_key);
    if(NULL == p_fea_acl)
    {
        return PM_E_NOT_EXIST;
    }

    /* get table TBL_CLEAR_ACL_POLICY */
    p_clear_acl_policy = tbl_clear_acl_policy_get_clear_acl_policy();

    /* sync with fea */
    p_clear_acl_policy->counter_id = p_fea_acl->counter_id;
    tbl_clear_acl_policy_set_clear_acl_policy_field(p_clear_acl_policy, TBL_CLEAR_ACL_POLICY_FLD_COUNTER_ID);

    /* sync aclid to clear flow policer stats */
    p_clear_acl_policy->acl_id = acl_id;
    tbl_clear_acl_policy_set_clear_acl_policy_field(p_clear_acl_policy, TBL_CLEAR_ACL_POLICY_FLD_ACL_ID);

    return PM_E_NONE;
}

int32
_acl_policy_clear_statistics_by_policy_map(char *policy_map_name, tbl_interface_t *p_tbl_if, bool is_input)
{
    /* class-map */
    tbl_class_in_policy_config_master_t *p_class_in_policy_master    = tbl_class_in_policy_config_get_master();
    tbl_class_in_policy_config_t        *p_db_class_in_policy_config = NULL;
    ctclib_slistnode_t                  *class_in_policy_listnode    = NULL;
    ctclib_slistnode_t                  *class_in_policy_next        = NULL;

    /* acl */
    tbl_acl_in_class_config_master_t *p_acl_in_class_master    = tbl_acl_in_class_config_get_master();
    tbl_acl_in_class_config_t        *p_db_acl_in_class_config = NULL;
    ctclib_slistnode_t               *acl_in_class_listnode    = NULL;
    ctclib_slistnode_t               *acl_in_class_next        = NULL;

    /* ace */
    tbl_ace_config_master_t *p_ace_config_master = tbl_ace_config_get_master();
    tbl_ace_config_t        *p_db_ace_config     = NULL;
    ctclib_slistnode_t      *ace_config_listnode = NULL;
    ctclib_slistnode_t      *ace_config_next     = NULL;
    

    /* other */
    int32  ret             = PM_E_NONE;
    uint64 class_priority  = 1;
    uint64 acl_priority    = 1;
    char   *class_map_name = "";
    char   *acl_name       = "";

    /* get all class-map in $policy_map_name */
    CTCLIB_SLIST_LOOP_DEL(p_class_in_policy_master->class_in_policy_config_list, p_db_class_in_policy_config, class_in_policy_listnode, class_in_policy_next)
    {
        if(0 == sal_strcmp(p_db_class_in_policy_config->key.policy_name, policy_map_name)) 
        {
            class_map_name = p_db_class_in_policy_config->key.class_name;
            class_priority = p_db_class_in_policy_config->class_priority;
            /* get all acl in $class_map_name */
            CTCLIB_SLIST_LOOP_DEL(p_acl_in_class_master->acl_in_class_config_list, p_db_acl_in_class_config, acl_in_class_listnode, acl_in_class_next)
            {
                if(0 == sal_strcmp(p_db_acl_in_class_config->key.class_name, class_map_name))
                {
                    acl_name     = p_db_acl_in_class_config->key.acl_name;
                    acl_priority = p_db_acl_in_class_config->acl_priority;
                    /* get all ace in $acl_name */
                    CTCLIB_SLIST_LOOP_DEL(p_ace_config_master->ace_config_list, p_db_ace_config, ace_config_listnode, ace_config_next)
                    {
                        if(0 == sal_strcmp(p_db_ace_config->key.acl_name, acl_name))
                        {
                            /* check time-range invalid ace bug 41516 */
                            if (GLB_FLAG_ISSET(p_db_ace_config->key_flags, GLB_IP_ACE_INVALID))
                            {
                                continue;
                            }
                            /* clear ace stats */
                            ret = _acl_policy_clear_statistics_by_ace(class_priority, acl_priority, p_tbl_if, p_db_ace_config, is_input);
                            if(ret)
                            {
                                return ret; 
                            }
                        }
                    }
                }
            }
        }
    }

    return PM_E_NONE;
}

int32
acl_policy_clear_statistics(char *ifname, bool is_input, cfg_cmd_para_t *para)
{
    int32 ret = 0;
    char *policy_map_name = "";
    tbl_interface_t *p_tbl_if = NULL;

    /* get interface table */
    p_tbl_if = tbl_interface_get_interface_by_name(ifname);
    if (NULL == p_tbl_if)
    {
        CFG_INVALID_PARAM_RET("Interface %s is not exist", ifname);
    }

    /* get policy-map name */
    policy_map_name = is_input ? p_tbl_if->igs_policy_map : p_tbl_if->egs_policy_map;
    if (NULL != policy_map_name && '\0' == *policy_map_name)
    {
        CFG_INVALID_PARAM_RET("Can't find policy-map on %s", ifname);
    }
    
    /* clear stats by policy-map */
    ret = _acl_policy_clear_statistics_by_policy_map(policy_map_name, p_tbl_if, is_input);

    return ret;
}

int32
acl_is_redirected_interface(tbl_interface_t *p_db_if)
{
    tbl_class_map_action_config_master_t *p_master = tbl_class_map_action_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_class_map_action_config_t *p_db_action = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->class_map_action_config_list, p_db_action, listnode)
    {
        if (p_db_if->ifindex == p_db_action->redirect_port_ifindex)
        {
            return PM_E_EXIST;
        }
    }

    return PM_E_NONE;
}
    
int32
acl_policy_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_POLICY_MAP_CONFIG, policy_map_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_CLASS_MAP_CONFIG, class_map_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_CLASS_IN_POLICY_CONFIG, class_in_policy_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_ACL_IN_CLASS_CONFIG, acl_in_class_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_CLASS_MAP_ACTION_CONFIG, class_map_action_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_CLEAR_ACL_POLICY, clear_acl_policy_cmd_process);

    cdb_register_cfg_act_cb(ACT_SHOW_ACL_POLICY_COUNTERS, acl_cmd_show_acl_policy_stats); 
    cdb_register_cfg_act_cb(ACT_SHOW_POLICY_MAP, acl_cmd_show_policy_map); 
    cdb_register_cfg_act_cb(ACT_SHOW_CLASS_MAP, acl_cmd_show_class_map); 

    if_agg_destory_agg_sync_prop_set_cb(IF_AGG_CB_MODULE_ACL_POLICY, acl_policy_agg_destroy);

    return PM_E_NONE;
}

