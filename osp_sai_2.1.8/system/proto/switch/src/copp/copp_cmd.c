
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_copp_cfg_define.h"
#include "gen/tbl_copp_cfg.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "copp.h"

extern int32
acl_policy_operation_copp_pmap_add_action_check(char* policy_map_name);

extern int32
policy_map_attach_interface(char *policy_map_name, char *interface_name, bool direct_in);

extern int32
policy_map_detach_interface(char *policy_map_name, char *interface_name, bool direct_in);


static int32
_copp_cmd_global_get(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_copp_cfg_t *p_copp_glb = tbl_copp_cfg_get_copp_cfg();
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;    
    tbl_copp_cfg_dump_one(p_copp_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_copp_cmd_check_pmap(char* pmap_name, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    if (0 != check_name_character_and_len(pmap_name, PMAP_NAME_SIZE))
    {
        CFG_INVALID_PARAM_RET("The character of policy-map name should be a-z, A-Z, or 0-9. The max policy map name len is 20.");
    }

    rc = acl_policy_operation_copp_pmap_add_action_check(pmap_name);
    /*modified by yejl for bug 49938 and bug 49973, 2018-11-13*/
    if (PM_E_CHECK_CMAP_MONITOR_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because mirror to session action does not support on copp");    
    }
    else if (PM_E_CHECK_CMAP_POLICY_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because policy action does not support on copp");
    }
    else if (PM_E_CHECK_CMAP_CTAG_COS_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because set c-vlan cos action does not support on copp");    
    }
    else if (PM_E_CHECK_CMAP_CTAG_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because set c-vlan action does not support on copp");
    }
    else if (PM_E_CHECK_CMAP_DSCP_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because set dscp action does not support on copp");
    }
    else if (PM_E_CHECK_CMAP_STAG_COS_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because set s-vlan cos action does not support on copp");
    }
    else if (PM_E_CHECK_CMAP_STAG_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because set s-vlan action does not support on copp");
    }
    else if (PM_E_CHECK_CMAP_TC_TO_COLOR_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because set traffic-class to color action does not support on copp");
    }
    else if (PM_E_CHECK_CMAP_REDIRECT_UNSUPPORT == rc)
    {
        CFG_INVALID_PARAM_RET("Failed because redirect to interface action does not support on copp");
    }
    /*end*/
    return PM_E_NONE;
}

static int32
_copp_cmd_global_set_total_rate(uint32 rate, cfg_cmd_para_t *para)
{
    tbl_copp_cfg_t *p_db_glb = tbl_copp_cfg_get_copp_cfg();
    int32 rc = PM_E_NONE;

    if (rate == p_db_glb->copp_total_rate)
    {
        return PM_E_NONE;
    }
    PM_E_RETURN(copp_cfg_global_set_total_rate(rate));
    return PM_E_NONE;
}


static int32
_copp_cmd_global_set_pmap(char* pmap_name, cfg_cmd_para_t *para)
{
    tbl_copp_cfg_t *p_db_glb = tbl_copp_cfg_get_copp_cfg();
    tbl_interface_t *pst_interface = NULL;
    int32 rc = PM_E_NONE;
    uint32 stm_max_ingress = 0;
    uint32 stm_max_egress = 0;

    /* 1. config check */
    PM_E_RETURN(_copp_cmd_check_pmap(pmap_name, para));
    
    /* 2. check duplicate config */
    if (0 == sal_strncmp(p_db_glb->copp_pname, pmap_name, sizeof(p_db_glb->copp_pname)))
    {
        return PM_E_NONE;
    }
    if (0 != sal_strlen(p_db_glb->copp_pname) && 
        0 != sal_strncmp(p_db_glb->copp_pname, pmap_name, sizeof(p_db_glb->copp_pname)))
    {
        CFG_INVALID_PARAM_RET("Copp has already configured policy map %s", p_db_glb->copp_pname);
    }

    pst_interface = tbl_interface_get_interface_by_name(GLB_IFNAME_CPU_PREFIX);
    if (NULL == pst_interface)
    {
        CFG_INVALID_PARAM_RET("Can not find copp interface");
    }
    
    /* 3. call API */
    rc = policy_map_attach_interface(pmap_name, pst_interface->key.name, TRUE); 
    /*added by yejl for bug 51969, 2019-05-30*/
    if (PM_E_INVALID_PARAM == rc)
    {
        CFG_INVALID_PARAM_RET("The initial character of acl name should be a-z, A-Z, or 0-9. The max policy-map name len is 20.");
    }
    else if (PM_E_PMAP_NOT_EXIST == rc)
    {
        CFG_INVALID_PARAM_RET("Policy map %s should be configured first", pmap_name);
    }
    else if (PM_E_CFG_NO_RESOURCE == rc)
    {
        if (check_feature_support(CONFIG_FEATURE_ACL_TCAM_RES_SHARE))
		{
        	/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
            stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
            stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
            CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress entry number is %d/%d",
                                    stm_max_ingress, stm_max_egress);
		}
        else
		{
			/* modified by huwx for bug 39772, acl stats id only support 1-4095 */
            stm_max_ingress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_INGRESS);
            stm_max_egress = lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW_ENTRIES_EGRESS);
            CFG_INVALID_PARAM_RET("The max hardware ACL ingress/egress:ingress6/egress6 entry number is %d/%d:%d/%d",
                                    stm_max_ingress, stm_max_egress,
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_INGRESS),
                                    lib_spec_get_field(TBL_SYS_SPEC_FLD_FLOW6_ENTRIES_EGRESS));
		}
    }
    else if (PM_E_CFG_POLICER_NO_RESOURCE == rc)
    {
        CFG_INVALID_PARAM_RET("QoS ingress flow policer profile resource full!");
    }
    else if (PM_E_CFG_ACL_TCP_NO_RESPURCE == rc)
    {
        CFG_INVALID_PARAM_RET("Exceeded maximum tcp flags entry number 4");
    }
    else if (PM_E_NONE != rc)
    {
        CFG_INVALID_PARAM_RET("Policy map configure failed");
    }
    /*ended by yejl*/

    return rc;

}

static int32
_copp_cmd_global_unset_pmap(cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_interface_t *pst_interface = NULL;
    tbl_copp_cfg_t *p_db_glb = tbl_copp_cfg_get_copp_cfg();

    /* 1. config check */
    if (0 == sal_strcmp(p_db_glb->copp_pname, ""))
    {
        return PM_E_NONE;
    }

    pst_interface = tbl_interface_get_interface_by_name(GLB_IFNAME_CPU_PREFIX);
    if (NULL == pst_interface)
    {
        CFG_INVALID_PARAM_RET("Can not find copp interface");
    }
    
    /* 2. call API */
    rc = policy_map_detach_interface(p_db_glb->copp_pname, pst_interface->key.name, TRUE);
    if (PM_E_PMAP_NOT_EXIST == rc)
    {
        CFG_INVALID_PARAM_RET("Policy map %s should be configured first", p_db_glb->copp_pname);
    }
    else if (PM_E_NONE != rc)
    {
        CFG_INVALID_PARAM_RET("Policy map configure failed");
    }

    return rc;
}


int32
copp_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    uint32 i = 0;
    uint32 value = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_COPP_CFG);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:

        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[0], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[0]);
        }

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        
        switch (field_id)
        {
        case TBL_COPP_CFG_FLD_COPP_PNAME:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
            if (0 == sal_strcmp(argv[1], "add"))
            {
                rc = _copp_cmd_global_set_pmap(argv[2] ,para);
            }
            else
            {
                rc = _copp_cmd_global_unset_pmap(para);
            }
            break;

        case TBL_COPP_CFG_FLD_COPP_TOTAL_RATE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT_RANGE(value, 0, GLB_CPU_TRAFFIC_MAX_RATE, argv[1], para->p_rs);
            rc = _copp_cmd_global_set_total_rate(value, para);
            if (0==value)
            {
                CFG_PROMPT_RET("WARNING:The configured rate is lower than total rate granularity.\n"
                                    "So the packet will be dropped");  
            }
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _copp_cmd_global_get(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

