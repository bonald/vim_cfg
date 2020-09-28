#include "proto.h"
#include "gen/tbl_class_map_config_define.h"
#include "gen/tbl_class_map_config.h"
#include "gen/tbl_acl_in_class_config_define.h"
#include "gen/tbl_acl_in_class_config.h"
#include "gen/tbl_policy_map_config_define.h"
#include "gen/tbl_policy_map_config.h"
#include "gen/tbl_class_in_policy_config_define.h"
#include "gen/tbl_class_in_policy_config.h"
#include "gen/tbl_class_map_action_config_define.h"
#include "gen/tbl_class_map_action_config.h"


int32
_ccs_cfg_build_acl_in_class_config(tbl_acl_in_class_config_t *p_acl_in_class_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    char *class_name = pargs->argv[1];;

    if (0 != sal_strcmp(p_acl_in_class_config->key.class_name, class_name))
    {
        return PM_E_NONE;
    }

    sal_fprintf (cfg_fp, " match access-list %s", p_acl_in_class_config->key.acl_name);
    
    sal_fprintf (cfg_fp, "\n");
    return PM_E_NONE;
}

int32
_ccs_cfg_build_class_map_config(tbl_class_map_config_t *p_class_map_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    tbl_iter_args_t args;

    sal_fprintf (cfg_fp, "class-map %s\n", p_class_map_config->key.name);


    if (0 == p_class_map_config->acl_ref)
    {
        sal_fprintf (cfg_fp, "!\n");
    }
    else
    {
        args.argv[0] = (void *)cfg_fp;
        args.argv[1] = (void *)(p_class_map_config->key.name);
        tbl_acl_in_class_config_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_acl_in_class_config, &args);
        sal_fprintf (cfg_fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_class_map(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;

    tbl_class_map_config_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_class_map_config, &args);
    return PM_E_NONE;
}

int32
_ccs_cfg_build_class_in_policy_config(tbl_class_in_policy_config_t *p_class_in_policy_config, tbl_iter_args_t *pargs)
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

    sal_fprintf (cfg_fp, " class %s", p_class_in_policy_config->key.class_name);
    sal_fprintf (cfg_fp, "\n");
    pst_class_map_action_config = tbl_class_map_action_config_get_class_map_action_config_by_name(p_class_in_policy_config->key.policy_name, p_class_in_policy_config->key.class_name);
    if(NULL != pst_class_map_action_config)
    {
        /*modified by yejl for bug 49954, 2018-11-12*/
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_MIRROR_SESSION))
        {
            sal_fprintf (cfg_fp, "  monitor to session %d", pst_class_map_action_config->session_id);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_POLICER))
        {
            sal_fprintf (cfg_fp, "  policer %s", pst_class_map_action_config->policer);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CCOS_VALUE))
        {
            sal_fprintf (cfg_fp, "  set ccos %d", pst_class_map_action_config->new_ccos);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_CVLAN_ID))
        {
            sal_fprintf (cfg_fp, "  set cvid %d", pst_class_map_action_config->new_cvlan_id);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_DSCP))
        {
            sal_fprintf (cfg_fp, "  set dscp %d", pst_class_map_action_config->new_dscp);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SCOS_VALUE))
        {
            sal_fprintf (cfg_fp, "  set scos %d", pst_class_map_action_config->new_scos);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_NEW_SVLAN_ID))
        {
            sal_fprintf (cfg_fp, "  set svid %d", pst_class_map_action_config->new_svlan_id);
            sal_fprintf (cfg_fp, "\n");
        }
        /* SYSTEM MODIFIED BEGIN: fix gb tc+color bug add by wangqj at 2016/8/11 */
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_TC))
        {
//            sal_fprintf (cfg_fp, "  set tc %d", pst_class_map_action_config->new_tc);
//            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_COLOR))
        {
            sal_fprintf (cfg_fp, "  set tc %d color %s", pst_class_map_action_config->new_tc, color_str[pst_class_map_action_config->new_color]);
            sal_fprintf (cfg_fp, "\n");
        }
        if(GLB_FLAG_ISSET(pst_class_map_action_config->action_flag, GLB_ACL_SET_REDIRECT_VALUE))
        {
            p_interface = tbl_interface_get_interface_by_ifindex(pst_class_map_action_config->redirect_port_ifindex);
            if (p_interface)
            {
                IFNAME_ETH2FULL(p_interface->key.name, ifname_ext);
                sal_fprintf (cfg_fp, "  redirect %s", ifname_ext);
                sal_fprintf (cfg_fp, "\n");
            }
        }
        else if ('\0' != pst_class_map_action_config->red_unexist_agg[0])
        {
            sal_fprintf (cfg_fp, "  redirect %s", pst_class_map_action_config->red_unexist_agg);
            sal_fprintf (cfg_fp, "\n");
        }
        /* END: fix gb tc+color bug add by wangqj at 2016/8/11 */ 
    }
    sal_fprintf (cfg_fp, "  exit\n");
    return PM_E_NONE;
}



int32
_ccs_cfg_build_policy_map_config(tbl_policy_map_config_t *p_policy_map_config, tbl_iter_args_t *pargs)
{
    FILE* cfg_fp = pargs->argv[0];
    tbl_iter_args_t args;

    sal_fprintf (cfg_fp, "policy-map %s\n", p_policy_map_config->key.name);


    if (0 == p_policy_map_config->class_ref)
    {
        sal_fprintf (cfg_fp, "!\n");
    }
    else
    {
        args.argv[0] = (void *)cfg_fp;
        args.argv[1] = (void *)(p_policy_map_config->key.name);
        tbl_class_in_policy_config_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_class_in_policy_config, &args);
        sal_fprintf (cfg_fp, "!\n");
    }

    return PM_E_NONE;
}


int32
ccs_cfg_build_policy_map(FILE* fp, char *key, cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = fp;

    tbl_policy_map_config_iterate((TBL_ITER_CB_FUNC)_ccs_cfg_build_policy_map_config, &args);
    return PM_E_NONE;
}


