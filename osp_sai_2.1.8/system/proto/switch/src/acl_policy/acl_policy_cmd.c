#include "proto.h"
#include "glb_acl_define.h"
#include "gen/tbl_policy_map_config_define.h" 
#include "gen/tbl_policy_map_config.h" 
#include "gen/tbl_class_map_config_define.h" 
#include "gen/tbl_class_map_config.h"
#include "gen/tbl_acl_in_class_config_define.h" 
#include "gen/tbl_acl_in_class_config.h" 
#include "gen/tbl_class_in_policy_config_define.h" 
#include "gen/tbl_class_in_policy_config.h" 
#include "gen/tbl_class_map_action_config_define.h" 
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "switch.h"
#include "acl_policy.h"



int32
class_map_cmd_read_class_map_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_class_map_config_iterate((TBL_ITER_CB_FUNC)tbl_class_map_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
policy_map_cmd_read_policy_map_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_policy_map_config_iterate((TBL_ITER_CB_FUNC)tbl_policy_map_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
class_in_policy_cmd_read_class_in_policy_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_class_in_policy_config_iterate((TBL_ITER_CB_FUNC)tbl_class_in_policy_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
acl_in_class_cmd_read_acl_in_class_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_acl_in_class_config_iterate((TBL_ITER_CB_FUNC)tbl_acl_in_class_config_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
policy_map_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *policy_map_name       = NULL;
    int rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_POLICY_MAP_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        policy_map_name = argv[0];

        rc = policy_map_config_add_policy_map(policy_map_name);
        if (PM_E_INVALID_PARAM == rc)
        {
            CFG_CONFLICT_RET("The initial character of policy-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if(PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The max number of policy map configure is %d.", ACL_POLICY_MAP_CONFIG_NUMBER);
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Policy-map create failed");
        }
        
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        policy_map_name = argv[0];

        rc = policy_map_config_del_policy_map(policy_map_name);
        if (PM_E_INVALID_PARAM == rc)
        {
            CFG_CONFLICT_RET("The initial character of policy-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_PMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Policy-map %s should be configured first", policy_map_name);
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Policy-map delete failed");
        }
        break;

    case CDB_OPER_GET:
        policy_map_cmd_read_policy_map_all(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
class_map_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *class_map_name       = NULL;
    int rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_CLASS_MAP_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        class_map_name = argv[0];

        rc = class_map_config_add_class_map(class_map_name);
        if (PM_E_INVALID_PARAM == rc)
        {
            CFG_CONFLICT_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The max number of class map configure is %d.", ACL_CLASS_MAP_CONFIG_NUMBER);
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Class-map create failed");
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        class_map_name = argv[0];

        rc = class_map_config_del_class_map(class_map_name);
        if (PM_E_INVALID_PARAM == rc)
        {
            CFG_CONFLICT_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_CMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Class-map %s should be configured first", class_map_name);
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Class-map delete failed");
        }
        break;

    case CDB_OPER_GET:
        class_map_cmd_read_class_map_all(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
class_in_policy_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *policy_map_name       = NULL;
    char *class_map_name       = NULL;
    int rc = PM_E_NONE;
    uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
    
    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_CLASS_IN_POLICY_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    if (para->oper == CDB_OPER_ADD || 
        para->oper == CDB_OPER_DEL)
    {
        policy_map_name = argv[0];
        class_map_name = argv[2];
        
        if (NULL == policy_map_name || NULL == class_map_name)
        {
            CFG_INVALID_PARAM_RET("Invalid arg");
        }
    }

    switch (para->oper)
    {
    case CDB_OPER_ADD:  
        rc = class_attach_policy(policy_map_name, class_map_name, para);
        if (PM_E_NOT_PERMIT == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of policy-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_PMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Policy map %s should be configured first", policy_map_name);
        }
        else if (PM_E_CMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Class map %s should be configured first", class_map_name);
        }
        else if(PM_E_CFG_NO_RESOURCE == rc)
        {
            if(check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
            {
                CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
                                    stm_max_ingress, stm_max_egress);
                
            }
            else
            {
                CFG_INVALID_PARAM_RET("The max hardware ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
                                    stm_max_ingress, 
                                    stm_max_egress,
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
            }
        }
        else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
        {
            CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
        }
        else if (PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The max number of class under one policy map is 1023");
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Class-map applied failed");
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        rc = class_detach_policy(policy_map_name, class_map_name);
        if (PM_E_NOT_PERMIT == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of policy-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_PMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Policy map %s should be configured first", policy_map_name);
        }
        else if (PM_E_CMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Class map %s should be configured first", class_map_name);
        }
        else if (PM_E_CLASS_IN_POLICY_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Class map %s should be applied in the policy map first", class_map_name);
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Class-map dis-applied failed");
        }
        break;

    case CDB_OPER_GET:
        class_in_policy_cmd_read_class_in_policy_all(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
acl_in_class_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    char *class_map_name       = NULL;
    char *acl_group_name       = NULL;
    int rc = PM_E_NONE;
    uint32 stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
    uint32 stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
    
    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_CLASS_IN_POLICY_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    if (para->oper == CDB_OPER_ADD || 
        para->oper == CDB_OPER_DEL)
    {
        class_map_name = argv[0];
        acl_group_name = argv[2];

        if (NULL == acl_group_name || NULL == class_map_name)
        {
            CFG_INVALID_PARAM_RET("Invalid arg");
        }
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD: 
        rc = acl_attach_class(class_map_name, acl_group_name, para);
        if (PM_E_NOT_PERMIT == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of acl name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_CMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Class map %s should be configured first", class_map_name);
        }
        else if (PM_E_ACL_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Acl %s should be configured first", acl_group_name);
        }
        else if (PM_E_CFG_NO_RESOURCE == rc)
        {
            if(check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
            {
                CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
                                    stm_max_ingress, stm_max_egress);
                
            }
            else
            {
                CFG_INVALID_PARAM_RET("The max hardware ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
                                    stm_max_ingress, 
                                    stm_max_egress,
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
            }
        }
        else if (PM_E_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("The acl has been existed in the class map");
        }
        else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
        {
            CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
        }
        else if (PM_E_RESOURCE_FULL == rc)
        {
            CFG_INVALID_PARAM_RET("The max number of acl under one class map is 63");
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Acl in class-map applied failed");
        }
        break;

    case CDB_OPER_SET:
        break;

    case CDB_OPER_DEL:
        rc = acl_detach_class(class_map_name, acl_group_name);
        if (PM_E_NOT_PERMIT == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of acl name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_CMAP_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Class map %s should be configured first", class_map_name);
        }
        else if (PM_E_ACL_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("ACL %s should be configured first", acl_group_name);
        }
        else if (PM_E_ACL_IN_CLASS_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("ACL %s should be applied in the class map first", acl_group_name);
        }
        else if (PM_E_NONE != rc)
        {
            CFG_INVALID_PARAM_RET("Acl in class-map dis-applied failed");
        }
        break;

    case CDB_OPER_GET:
        acl_in_class_cmd_read_acl_in_class_all(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


#define GLB_ACTION_MAP_TYPE_TO_STR(_type_) \
    (_type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_TC      ?  "tc"        : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_COLOR   ?  "color"           : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_DSCP    ?  "dscp"           : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SCOS    ?  "scos"           : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CCOS    ?  "ccos"            : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_SVLAN_ID ?  "svlan"            : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_NEW_CVLAN_ID ?  "cvlan"   : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_REDIRECT_PORT_IFINDEX ?  "redirect"          : \
     _type_ == TBL_CLASS_MAP_ACTION_CONFIG_FLD_SESSION_ID   ?  "mirror"            : \
     "this")

int32
class_map_action_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    int rc = PM_E_NONE;
    char *class_map_name       = NULL;
    char *policy_map_name       = NULL;
    cdb_field_t *p_field = NULL;
    int32 field_id = 0;

    CFG_DBG_FUNC();
    policy_map_name = argv[0];
    class_map_name = argv[2];
    if (NULL == class_map_name || NULL == policy_map_name)
    {
        if(0 != argc)
        {
            CFG_INVALID_PARAM_RET("Invalid arg");
        }
    }
    
    p_node = cdb_get_tbl(TBL_CLASS_MAP_ACTION_CONFIG);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    field_id = cdb_get_field_id(argv[3], p_node, &p_field);   
    switch (para->oper)
    {
    case CDB_OPER_ADD:   
        break;

    case CDB_OPER_SET:
        rc = class_map_action_update(policy_map_name, class_map_name, field_id, argv[4], para);
        if (PM_E_NOT_PERMIT == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of policy-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_INVALID_PARAM == rc)
        {
            CFG_INVALID_PARAM_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
        }
        else if (PM_E_NOT_EXIST == rc)
        {
            CFG_INVALID_PARAM_RET("Policy map %s should be configured first", policy_map_name);
        }
        else if (PM_E_CHECK_CMAP_POLICY_APPLY_ON_EGRESS_PORT == rc)
        {
            CFG_INVALID_PARAM_RET("Failed to set the action because the policy map has been configured on output interface");
        }
        else if (PM_E_CHECK_CMAP_POLICY_APPLY_ON_COPP == rc)
        {
            CFG_INVALID_PARAM_RET("Failed to set the action because the policy map has been configured on copp");
        }
        else if (PM_E_CFG_POLICER_NO_RESOURCE == rc)
        {
            CFG_INVALID_PARAM_RET("Flow policer profile resource full");
        }
        else if (PM_E_NONE != rc)
        {
            if(0 == sal_strlen(para->p_rs->desc))
            {
                CFG_INVALID_PARAM_RET("Class map update action failed");
            }
            else
            {
                return rc;
            }
        }
        break;

    case CDB_OPER_DEL:

        break;

    case CDB_OPER_GET:
        rc = class_map_action_show(para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}


int32
clear_acl_policy_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node   = NULL;
    int32 rc = PM_E_NONE;
    bool is_input = TRUE;
    char *if_name = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CLEAR_ACL_POLICY);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    /* check parameters, argv[0]: if_name, argv[1]: dir */
    if (argc < 2)
    {
        CFG_INVALID_PARAM_RET("The parameters is not completed");
    }
    else if('\0' == argv[0])
    {
        CFG_INVALID_PARAM_RET("Invalid interface name");
    }
    else if(0 != sal_strcmp("input", argv[1]) && 0 != sal_strcmp("output", argv[1])) 
    {
        CFG_INVALID_PARAM_RET("Invalid direction");
    }

    if_name = argv[0];
    is_input = (0 == sal_strcmp("input", argv[1])) ? TRUE : FALSE;

    switch(para->oper)
    {
        case CDB_OPER_SET:
            acl_policy_clear_statistics(if_name, is_input, para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unsupported action");
            break;
    }

    return rc;
}

int32
acl_cmd_show_acl_policy_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }
        rc = acl_policy_show_acl_policy(fp, argv, argc, para);
        sal_fclose(fp);
        fp = NULL;
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
acl_cmd_show_policy_map(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_policy_map_config_key_t pmap_config_key;
    tbl_policy_map_config_t *p_db_pmap_config = NULL;
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            if (check_name_character_and_len(argv[0], PMAP_NAME_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("The initial character of policy-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
            }
            sal_memset(&pmap_config_key, 0, sizeof(pmap_config_key));
            sal_strcpy(pmap_config_key.name, argv[0]);
            p_db_pmap_config = tbl_policy_map_config_get_policy_map_config(&pmap_config_key);
            if (NULL == p_db_pmap_config)
            {
                CFG_INVALID_PARAM_RET("Policy map %s does not exist", argv[0]);
            }
            rc = acl_policy_show_policy_map(p_db_pmap_config, para);
        }
        else
        {
            rc = acl_policy_show_policy_map(NULL, para);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
acl_cmd_show_class_map(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_class_map_config_key_t cmap_config_key;
    tbl_class_map_config_t *p_db_cmap_config = NULL;
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            if (check_name_character_and_len(argv[0], CMAP_NAME_SIZE) != 0)
            {
                CFG_CONFLICT_RET("The initial character of class-map name should be a-z, A-Z, or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20");
            }
            sal_memset(&cmap_config_key, 0, sizeof(cmap_config_key));
            sal_strcpy(cmap_config_key.name, argv[0]);
            p_db_cmap_config = tbl_class_map_config_get_class_map_config(&cmap_config_key);
            if (NULL == p_db_cmap_config)
            {
                CFG_INVALID_PARAM_RET("Class map %s does not exist", argv[0]);
            }
            rc = acl_policy_show_class_map(p_db_cmap_config, para);
        }
        else
        {
            rc = acl_policy_show_class_map(NULL, para);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}
